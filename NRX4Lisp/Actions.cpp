#include "pch.h"
#include  <stdio.h>
#include  "adslib.h"
#include  "rxregsvc.h"
#include  "rxdefs.h"
#include "tchar.h"

#include "Actions.hpp"
#include "MS_Actions.hpp"

/*
    Оригинальный код взят из примера ObjectARX 2023 ...\Samples\misc\fact_dg\fact.cpp
*/
#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))


/* First, define the structure of the table: a string giving the AutoCAD name
   of the function, and a pointer to a function returning type int. */
struct func_entry { TCHAR* func_name; int (*func) (struct resbuf*); };
/*-----------------------------------------------------------------------*/
int GetByRb(struct resbuf* rb, AcDbObjectId& id)
{
    //long id_old;
    auto type = rb->restype;
    //if (rb->restype == RTREAL) {
    //    id_old = rb->resval.rlong;          /* Save in local variable */
    //}
    //else {
    //    acdbFail(_T(/*MSG3*/"Argument should be an long."));
    //    return RTERROR;
    //}

    if (type == RTSTR)
    {
        auto id_old3 = rb->resval.rstring;

        AcDbHandle id_handle(id_old3);
        AcDbDatabase* pCurDb = acDocManager->curDocument()->database();

        if (pCurDb->getAcDbObjectId(id, false, id_handle) == Acad::eOk)
        {
            return RTNORM;
        }
        else return RTERROR;
    }
    //id.setFromOldId(id_old2);
    return RTERROR;
}
static int SaveElementToXml(struct resbuf* rb)
{
    AcDbObjectId id;
    int wait_getting = GetByRb(rb, id);
    if (wait_getting == RTERROR) {
        acdbFail(_T(/*MSG3*/"Argument is not valid"));
        return RTERROR;
    }

    /* acedRetReal(rfact(x));            Call the function itself, and
                                         return the value to AutoLISP */
    
    return SaveElementToXml_Impl(id);
}

static int Viper_GetAxisCoords(struct resbuf* rb) {
    AcDbObjectId id;
    int wait_getting = GetByRb(rb, id);
    if (wait_getting == RTERROR) {
        acdbFail(_T(/*MSG3*/"Argument is not valid"));
        return RTERROR;
    }

    return Viper_GetAxisCoords_Impl(id);
}

static int Cable_GetAxisCoords(struct resbuf* rb) {
    AcDbObjectId id;
    int wait_getting = GetByRb(rb, id);
    if (wait_getting == RTERROR) {
        acdbFail(_T(/*MSG3*/"Argument is not valid"));
        return RTERROR;
    }

    return Cable_GetAxisCoords_Impl(id);
}

/*-----------------------------------------------------------------------*/
/* Here we define the array of function names and handlers. */
static struct func_entry func_table[] = { 
    {_T(/*MSG0*/"ms_save_props_to_xml"), SaveElementToXml},
    {_T(/*MSG0*/"ms_viper_get_axis_coords"), Viper_GetAxisCoords},
    {_T(/*MSG0*/"ms_cable_get_axis_coords"), Cable_GetAxisCoords}
};

int Actions::LoadFunctions() {
    for (int i = 0; i < ELEMENTS(func_table); i++) {
        if (!acedDefun(func_table[i].func_name, i))
            return RTERROR;
    }
    return RTNORM;
}

/*-----------------------------------------------------------------------*/
/* RecognizeFunction -- Execute external function (called upon an RQSUBR request).
            Return value from the function executed, RTNORM or RTERROR. */
int Actions::RecognizeFunction()
{
    struct resbuf* rb;
    int val;

    /* Get the function code and check that it's within range.
       (It can't fail to be, but paranoia doesn't hurt.) */
    if ((val = acedGetFunCode()) < 0 || val >= ELEMENTS(func_table)) {
        acdbFail(_T(/*MSG2*/"Received nonexistent function code."));
        return RTERROR;
    }

    /* Fetch the arguments, if any. */
    rb = acedGetArgs();

    /* Call the handler and return its success-failure status. */
    val = (*func_table[val].func)(rb);
    acutRelRb(rb);
    return val;
}

