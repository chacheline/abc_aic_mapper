 /**CFile****************************************************************

  FileName    [cmRequired.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Cone mapping based on priority cuts.]

  Synopsis    [Required time calculation functions]

  Author      [Martin Thuemmler]
  
  Affiliation [TU Dresden]

  Date        [Ver. 1.0. Started - February 15, 2021.]

  Revision    [$Id: cmRequired.c,v 1.00 2021/02/15 00:00:00 thm Exp $]

***********************************************************************/


#include "cm.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Calculates the required times from CO to CI of 
               nodes visible for mapping.]

  Description []
               
  SideEffects [Node mark is reset and CM_MARK_VISIBLE is recalculated.]

  SeeAlso     []

***********************************************************************/
void Cm_ManCalcVisibleRequired(Cm_Man_t *p)
{
    float * AicDelay = p->pPars->AicDelay;
    int enumerator;
    Cm_Obj_t *pObj;
    Cm_ManForEachObj(p, pObj, enumerator)
    {
        pObj->fMark = 0;
        if ( pObj->Type == CM_CO )
        {
            pObj->fMark = CM_MARK_VISIBLE;
            pObj->pFanin0->fMark = CM_MARK_VISIBLE;
            pObj->pFanin0->Required = pObj->Required;
        }
        else 
        {
            pObj->Required = CM_FLOAT_LARGE;
        }
       
    }
    Cm_ManForEachObjReverse(p, pObj, enumerator)
    {
        if ( !(pObj->fMark & CM_MARK_VISIBLE) || pObj->Type != CM_AND )
            continue;
        float req = pObj->Required - AicDelay[pObj->BestCut.Depth];
        for(int i=0; i<pObj->BestCut.nFanins; i++){
            Cm_Obj_t *l = pObj->BestCut.Leafs[i];
            l->fMark |= CM_MARK_VISIBLE;
            if ( l->Required > req)
                l->Required = req;
        }
    }
}

/**Function*************************************************************

  Synopsis    [Sets the required times for invisible nodes.]

  Description [The required time is set for each node to the minimum
               value of all visible cuts it is contained in. ]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Cm_ManSetInvisibleRequired(Cm_Man_t *p)
{
    int enumerator;
    Cm_Obj_t * pObj;
    Cm_ManForEachObj(p, pObj, enumerator)
    {
        if( pObj->Type != CM_AND)
            continue;
        if ( !(pObj->fMark&CM_MARK_VISIBLE) )
            continue;
        for(int i=0; i<pObj->BestCut.nFanins; i++)
            pObj->BestCut.Leafs[i]->fMark |= CM_MARK_LEAF;
 
        Cm_Obj_t * pQueue[CM_MAX_FA_SIZE];
        pQueue[0] = pObj->pFanin0;
        pQueue[1] = pObj->pFanin1;
        int front = 0, back = 2;
        while(front != back)
        {
            // discard outside / invisible nodes
            Cm_Obj_t * pFront = pQueue[front++];
            // found deepest leaf? 
            if ( (pFront->fMark & CM_MARK_LEAF)  )
                continue;
            float req = pObj->Required; 

            if ( !(pFront->fMark & CM_MARK_VISIBLE) && pFront->Required > req)
                pFront->Required = req;

            if ( pFront->pFanin0 )
                pQueue[back] = pFront->pFanin0;
            if ( pFront->pFanin1 )
                pQueue[back] = pFront->pFanin1;
       }
       for(int i=0; i<pObj->BestCut.nFanins; i++)
           pObj->BestCut.Leafs[i]->fMark &= ~CM_MARK_LEAF;
    }
}

ABC_NAMESPACE_IMPL_END
