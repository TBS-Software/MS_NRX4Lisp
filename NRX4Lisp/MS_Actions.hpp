#pragma once
#include "sstream"
#include "CSDataInterfacesImpl.h"
#include "MStudioObject.h"
#include "vCSEntity.h"
#include "vCSSegment.h"
#include "vCSAxis.h"
#include "vCSNode.h"
#include "vCSNodeBase.h"

#include "mstRoutePrototype.h"

static std::wstring GenerateTempPath() {
	TCHAR lpTempPathBuffer[MAX_PATH];
	//  Generates a temporary file name. 
	auto dwRetVal = GetTempPath(MAX_PATH,// length of the buffer
		lpTempPathBuffer); // buffer for path 

	TCHAR szTempFileName[MAX_PATH];
	auto uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
		TEXT("MS_"),     // temp file name prefix 
		0,                // create unique name 
		szTempFileName);  // buffer for name
	return szTempFileName;
}


/// <summary>
/// Получает у объекта MS ElementData (CElement*) и сохраняет их в XML-файл
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
static int SaveElementToXml_Impl(AcDbObjectId& id) {

	AcDbEntity* sourceEntity;
	acdbOpenObject(sourceEntity, id, AcDb::kForRead);
	
	CMStudioObject* ms_object = reinterpret_cast <CMStudioObject*>(sourceEntity);
	if (ms_object)
	{
		CElement* ms_element = reinterpret_cast <CElement*>(ms_object);
		if (ms_element)
		{
			std::wstring write_path = GenerateTempPath();
			CString write_path_c = write_path.c_str();
			//bool check_1 = ms_element->WriteToXML(write_path_c);
			bool check_2 = ms_element->WriteToFile(write_path.c_str());

			acedPrompt(write_path_c);
		}
		
	}
	sourceEntity->close();
	return RTNORM;
}

/// <summary>
/// Приводит объект к классу vCSAxis и получает координаты её оборудования, создает на их основе 3d-полилинию
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
static int Viper_GetAxisCoords_Impl(AcDbObjectId& id) {

	acDocManager->unlockDocument(acDocManager->curDocument());
	AcDbDatabase* pCurDb = acDocManager->curDocument()->database();
	
	AcDbBlockTable* pBlockTable;
	acdbHostApplicationServices()->workingDatabase()
		->getSymbolTable(pBlockTable, AcDb::kForRead);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,
		AcDb::kForWrite);
	pBlockTable->close();

	AcDbEntity* sourceEntity;
	acdbOpenObject(sourceEntity, id, AcDb::kForRead);

	CMStudioObject* ms_object = reinterpret_cast <CMStudioObject*>(sourceEntity);
	if (ms_object)
	{
		vCSEntity* viper_object = reinterpret_cast <vCSEntity*>(ms_object);
		if (viper_object)
		{
			vCSAxis* viper_axis = reinterpret_cast <vCSAxis*>(ms_object);
			if (viper_axis)
			{
				AcDbObjectIdArray viper_nodes = viper_axis->GetNodesArr();
				int viper_nodes_count = viper_nodes.length();

				AcGePoint3dArray points3d;
				points3d.setLogicalLength(viper_nodes_count);

				int p_counter = 0;
				for (auto viper_node_id : viper_nodes) {
					AcDbEntity* viper_node_entity;
					acdbOpenObject(viper_node_entity, id, AcDb::kForRead);

					//BasePoint
					vCSImplNodeBase* viper_segment_object = reinterpret_cast <vCSImplNodeBase*>(viper_node_entity);
					if (viper_segment_object)
					{
						AcDbExtents extents;
						AcGePoint3d base_point_start = viper_segment_object->GetStartPoint();
						AcGePoint3d base_point_end = viper_segment_object->GetEndPoint();
						Nano::ErrorStatus er = viper_segment_object->_getGeomExtents(extents);
						//AcGePoint3d base_point = node->BasePoint();

						std::stringstream ss;
						ss << 
							base_point_start.x << " " << std::fixed <<
							base_point_start.y << " " << std::fixed <<
							base_point_start.z << " " << std::endl << std::fixed <<
							base_point_end.x << " " << std::fixed <<
							base_point_end.y << " " << std::fixed <<
							base_point_end.z << " " << std::endl << std::fixed <<
							extents.maxPoint().x << " " << std::fixed <<
							extents.maxPoint().y << " " << std::fixed <<
							extents.maxPoint().z << " " << std::endl << std::fixed <<
							extents.minPoint().x << " " << std::fixed <<
							extents.minPoint().y << " " << std::fixed <<
							extents.minPoint().z << " " << std::endl;

						CA2W ca2w(ss.str().c_str());
						std::wstring wide = ca2w;
						acedPrompt(wide.c_str());
						ss.str("");
						//AcGePoint3d base_point = viper_segment_object->BasePoint();
						points3d.setAt(p_counter, base_point_end);
						p_counter++;
					}

				}

				AcDb3dPolyline* pline3d = new AcDb3dPolyline(AcDb::k3dSimplePoly, points3d, 0);
				AcDbObjectId acPolylineId;
				pBlockTableRecord->appendAcDbEntity(acPolylineId, pline3d);
				pline3d->close();
			}
		}
	}
	acDocManager->unlockDocument(acDocManager->curDocument());
	return RTNORM;
 }

/// <summary>
/// Обновляет элемент после изменений
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
static int UpdateElementsViper_Impl(AcDbObjectId& id){
	AcDbEntity* sourceEntity;
	acdbOpenObject(sourceEntity, id, AcDb::kForWrite);

	CMStudioObject* ms_object = reinterpret_cast <CMStudioObject*>(sourceEntity);
	if (ms_object) {
		vCSEntity* viper_object = reinterpret_cast <vCSEntity*>(ms_object);
		if (viper_object)
		{
			viper_object->updateElements(true);
		}
	}
	//TODO: реализовать также для vCSAxis SetPosition* (альтернативыный метод для обновления на уровне оси)
	sourceEntity->close();
}

/// <summary>
/// Получает точки из трассы MS Кабельного хозяйства и выводит их в консоль
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
static int Cable_GetAxisCoords_Impl(AcDbObjectId& id) {
	AcDbEntity* sourceEntity;
	acdbOpenObject(sourceEntity, id, AcDb::kForRead);

	CMStudioObject* ms_object = reinterpret_cast <CMStudioObject*>(sourceEntity);
	if (ms_object)
	{
		mstRoutePrototype* cable_object = reinterpret_cast <mstRoutePrototype*>(ms_object);
		if (cable_object)
		{
			std::shared_ptr<AcGeCurve3d> axis = cable_object->getAxis();
			AcGePoint3d end_point;
			if (axis->hasEndPoint(end_point)) {
				double end_point_param = axis->paramOf(end_point);
				for (int point_counter = 0; point_counter < end_point_param; point_counter++)
				{
					AcGePoint3d step_point = axis->evalPoint(point_counter * 1.0);
					std::stringstream ss;
					ss << std::fixed <<
						step_point.x << " " << std::fixed <<
						step_point.y << " " << std::fixed <<
						step_point.z << " " << std::endl;

					CA2W ca2w(ss.str().c_str());
					std::wstring wide = ca2w;
					acedPrompt(wide.c_str());
					ss.str("");
				}
			}	
		}
	}
	acDocManager->unlockDocument(acDocManager->curDocument());
	return RTNORM;
}