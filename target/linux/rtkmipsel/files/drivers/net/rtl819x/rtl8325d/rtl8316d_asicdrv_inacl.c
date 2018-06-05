/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision:  $
* $Date: $
*
* Purpose : ASIC-level driver implementation for ingress Access Control List.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <rtk_api_ext.h>
#include <rtk_api.h>
#include <asicdrv/rtl8316d_types.h> /*specify the file in SDK ASIC driver directory*/
#include <rtl8316d_table_struct.h>
#include <rtl8316d_asicdrv_inacl.h>
//#include <rtl8316d_asicdrv_tbl.h>
#include <rtl8316d_debug.h>
#include <rtl8316d_reg_struct.h>

#if 1
#include <linux/gfp.h>
#include <linux/slab.h>

void *malloc(unsigned int NBYTES) {
	if(NBYTES==0) return NULL;
	return (void *)kmalloc(NBYTES,GFP_ATOMIC);
}

void free(void *APTR) {
	kfree(APTR);
}
#endif

/*ACLs in each ASIC PIE block is represented by a ACL group*/
rtk_filter_aclGroup_t pAclGroup[RTK_ACL_BLOCK_MAX];

/* Function Name:
 *      rtk_filter_igrAcl_init
 * Description:
 *      ACL module initialization.
 * Input:
 *     None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API initialized ACL modules. 
 */
rtk_api_ret_t rtk_filter_igrAcl_init(void)
{
    rtk_aclBlock_id_t blkId;

    for (blkId = 0; blkId < RTK_ACL_BLOCK_MAX; blkId++)
    {
        /*should remove ACL entries first*/
        pAclGroup[blkId].entryNum = 0;
        pAclGroup[blkId].aclEntry = NULL;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_reInit
 * Description:
 *      ACL module re-initialization
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API re-initialized ACL modules. 
 */
rtk_api_ret_t rtk_filter_igrAcl_reInit(void)
{
    rtk_aclBlock_id_t blkId;
    uint16 entryIdx, entryNum;
    rtk_filter_aclEntry_t* pAclEntry;
    rtk_api_ret_t retVal;
    
    for (blkId = 0; blkId < RTK_ACL_BLOCK_MAX; blkId++)
    {
        /*should remove ACL entries first*/
        if (pAclGroup[blkId].entryNum == 0){
            continue;
        } else {
            pAclEntry = pAclGroup[blkId].aclEntry;
            entryNum = pAclGroup[blkId].entryNum;
            for (entryIdx = 0; entryIdx < entryNum; entryIdx++, pAclEntry = pAclEntry->next)
            {   
                ASSERT(pAclEntry != NULL);
                retVal = rtk_filter_aclRule_remove(blkId, pAclEntry->ruleId, &pAclEntry);
                if(retVal != RT_ERR_OK)
                {
                    return retVal;
                }
                
                retVal = rtk_filter_aclEntry_free(pAclEntry);
                if(retVal != RT_ERR_OK)
                {
                    return retVal;
                }
            }
        }
        
        ASSERT(pAclGroup[blkId].entryNum == 0);
        ASSERT(pAclGroup[blkId].aclEntry == NULL);       
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_unmatchAction_set
 * Description:
 *      Set action to packets when no ACL rule match
 * Input:
 *      port - port ID
 *      action 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - error input port ID.
 *      RT_ERR_INPUT - Invalid input action.
 *      RT_ERR_SMI - SMI access error.
 * Note:
 *      This function sets action of packets when no ACL configruation matches.
 *      Valid action including:
 *          (1). FILTER_UNMATCH_PERMIT
 *          (2). FILTER_UNMATCH_DROP
 */
rtk_api_ret_t rtk_filter_unmatchAction_set(rtk_filter_port_t port, rtk_filter_unmatch_action_t action)
{
    rtk_api_ret_t ret;
    uint32 actVal;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if(action > FILTER_UNMATCH_END)
        return RT_ERR_INPUT;

    if (action == FILTER_UNMATCH_DROP)
        actVal = 1;
    else actVal = 0;
    ret = reg_field_write(RTL8316D_UNIT, PORT0_ACL_LOOKUP_CONTROL+port, DEFACT, actVal);

    return ret;
}

/* Function Name:
 *      rtk_filter_unmatchAction_get
 * Description:
 *      Get action to packets when no ACL rule match
 * Input:
 *      port - port ID
 * Output:
 *      pAction - action returned 
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 *      RT_ERR_PORT_ID - error input port ID.
 *      RT_ERR_SMI - SMI access error.
 * Note:
 *      This function gets action of packets when no ACL rule matches.
 *      Valid action including:
 *          (1). FILTER_UNMATCH_PERMIT
 *          (2). FILTER_UNMATCH_DROP
 */
rtk_api_ret_t rtk_filter_unmatchAction_get(rtk_filter_port_t port, rtk_filter_unmatch_action_t* pAction)
{
    rtk_api_ret_t ret;
    uint32 actVal;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (pAction == NULL)
        return RT_ERR_NULL_POINTER;
    
    ret = reg_field_read(RTL8316D_UNIT, PORT0_ACL_LOOKUP_CONTROL+port, DEFACT, &actVal);

    if (actVal == 1)
        *pAction = FILTER_UNMATCH_DROP;
    else *pAction = FILTER_UNMATCH_PERMIT;
    
    return ret;
}

/* Function Name:
 *      rtk_filter_igrAcl_apply
 * Description:
 *      write ACL rules into ASIC
 * Input:
  *      blockId  - ACL block ID
  * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API write ACL rules into ASIC. 
 */
rtk_api_ret_t rtk_filter_igrAcl_apply(rtk_aclBlock_id_t blockId)
{
//    uint16 entryIdx, entryNum;
    rtk_filter_aclEntry_t* pAclEntry;
    rtl8316d_aclEntry_t aclAsicEntry;
    rtk_filter_aclField_t* pField;
    rtk_filter_field_type_t fieldType;
    rtk_api_ret_t retVal;
    uint8 fieldMap[32];
    uint8 fieldIdx, i;

    for (fieldIdx = 0; fieldIdx < PIETEM_UNUSED; fieldIdx++)
    {
        fieldMap[fieldIdx] = RTL8316D_ACLFIELD_MAX;
    }
    
    for (fieldIdx = 0; fieldIdx < RTL8316D_ACLFIELD_MAX; fieldIdx++)
    {
        retVal = rtk_filter_templ_get(blockId, fieldIdx, &fieldType);
        if (retVal != RT_ERR_OK)
        {
            return retVal;
        }
        fieldMap[fieldType] = fieldIdx;        
    }

    for (pAclEntry = pAclGroup[blockId].aclEntry; pAclEntry != NULL; pAclEntry = pAclEntry->next)
    {
        memset(&aclAsicEntry, 0, sizeof(aclAsicEntry));
        for (pField = pAclEntry->pField; pField != NULL; pField = pField->next)
        {
            i = fieldMap[pField->type];
            aclAsicEntry.ruleField[i].fieldType = pField->type;
            memcpy(&aclAsicEntry.ruleField[i].data, &pField->fieldData, sizeof(rtk_filter_pieTempl_field_t));
            memcpy(&aclAsicEntry.ruleField[i].mask, &pField->fieldMask, sizeof(rtk_filter_pieTempl_field_t));
        }
        if (rtl8316d_inAclRule_setAsicEntry(pAclEntry->ruleId, &aclAsicEntry) != SUCCESS)
        {
            return RT_ERR_FILTER_RULEAPPLY_FAILED;
        }     
        aclAsicEntry.pAction = (rtl8316d_aclAct_entry_t*)pAclEntry->pAction;
        if (rtl8316d_actTbl_setAsicEntry(pAclEntry->ruleId, aclAsicEntry.pAction) != SUCCESS)
        {
            return RT_ERR_FILTER_ACTIONAPPLY_FAILED;
        }

        if (rtk_filter_reverseHit_set(blockId, pAclEntry->ruleId, pAclEntry->reverse) != SUCCESS)
        {
            return RT_ERR_FILTER_REVERSE;
        }
    }    

    return RT_ERR_OK;
}


int32 rtl8316d_actTbl_getAsicEntry(uint32 index, rtl8316d_aclAct_entry_t * actEntry_p)
{
    rtl8316d_tblasic_aclActTbl_t asicentry;
    rtl8316d_tblAsic_aclRedirect_t redirInfo;
    rtl8316d_tblAsic_aclOtagop_t otagInfo;
    rtl8316d_tblAsic_aclIntagop_t itagInfo;
    rtl8316d_tblAsic_aclDscpRmk_t dscpInfo;
    rtl8316d_tblAsic_aclPrioRmk_t prirmkInfo;
    rtl8316d_tblAsic_aclPriority_t priInfo;
    uint32 retvalue;
    uint32 infonum = 0;

    if ((actEntry_p == NULL) || (index >= RTL8316D_ACLACT_ENTRYNUM))
        return -1;

    retvalue = table_read(RTL8316D_UNIT, ACTION_TABLE, index, (uint32 *)&asicentry);

    if (retvalue != 0)
    {
        rtlglue_printf("read acl ation table entry %d failed\n", index);
        return -1;
    }

    actEntry_p->fno = asicentry.fno;
    actEntry_p->copytocpu = asicentry.copytocpu;
    actEntry_p->drop = asicentry.drop;
    actEntry_p->dscprmk = asicentry.dscprmk;
    actEntry_p->itag = asicentry.itag;
    actEntry_p->mirror = asicentry.mirror ;
    actEntry_p->otag = asicentry.otag;
    actEntry_p->prioasn = asicentry.prioasn;
    actEntry_p->priormk = asicentry.priormk;
    actEntry_p->redir = asicentry.redir;

    if (1 == asicentry.otag)
    {
        otagInfo = *(rtl8316d_tblAsic_aclOtagop_t *)&asicentry.actinfo[infonum];
        actEntry_p->outTagOpInfo.outTagOp = otagInfo.outTagOp;
        actEntry_p->outTagOpInfo.outVidCtl = otagInfo.outVidCtl;
        actEntry_p->outTagOpInfo.outVidInfo = otagInfo.outVidInfo;
        actEntry_p->outTagOpInfo.withdraw = otagInfo.withdraw;

        infonum++;
    }

    if (1 == asicentry.itag)
    {
        itagInfo = *(rtl8316d_tblAsic_aclIntagop_t *)&asicentry.actinfo[infonum];
        actEntry_p->inTagOpInfo.inTagOp = itagInfo.inTagOp;
        actEntry_p->inTagOpInfo.inVidCtl = itagInfo.inVidCtl;
        actEntry_p->inTagOpInfo.inVidInfo = itagInfo.inVidInfo;
        actEntry_p->inTagOpInfo.withdraw = itagInfo.withdraw;

        infonum++;
    }

    if (1 == asicentry.priormk)
    {
        prirmkInfo = *(rtl8316d_tblAsic_aclPrioRmk_t *)&asicentry.actinfo[infonum];
        actEntry_p->PriRmkInfo.dei = prirmkInfo.dei;
        actEntry_p->PriRmkInfo.inPri = prirmkInfo.inPri;
        actEntry_p->PriRmkInfo.outPri = prirmkInfo.outPri;
        actEntry_p->PriRmkInfo.tagSel = prirmkInfo.tagSel;
        actEntry_p->PriRmkInfo.withdraw = prirmkInfo.withdraw;

        infonum++;
    }

    if((1 == asicentry.redir) && (infonum<3))
    {
        redirInfo = *(rtl8316d_tblAsic_aclRedirect_t *)&asicentry.actinfo[infonum];
        if (0 == redirInfo.redirect.uniRedirect.opcode)
        {
            actEntry_p->redirInfo.opcode = redirInfo.redirect.uniRedirect.opcode;
            actEntry_p->redirInfo.withdraw = redirInfo.redirect.uniRedirect.withdraw;
            actEntry_p->redirInfo.redirect.uniRedirect.dpn = redirInfo.redirect.uniRedirect.dpn;
        }
        else
        {
            actEntry_p->redirInfo.opcode = redirInfo.redirect.multiRedirect.opcode;
            actEntry_p->redirInfo.withdraw = redirInfo.redirect.multiRedirect.withdraw;
            actEntry_p->redirInfo.redirect.multiRedirect.ftIdx = redirInfo.redirect.multiRedirect.ftIdx;
        }

        infonum++;
    }

    if ((1 == asicentry.dscprmk) && (infonum<3))
    {
        dscpInfo = *(rtl8316d_tblAsic_aclDscpRmk_t *)&asicentry.actinfo[infonum];
        if (0 == dscpInfo.dscpRmk.dscp.opcode)
        {
            actEntry_p->dscpRmkInfo.dscpRmk.dscp.acldscp = dscpInfo.dscpRmk.dscp.acldscp;
            actEntry_p->dscpRmkInfo.opcode = dscpInfo.dscpRmk.dscp.opcode;
            actEntry_p->dscpRmkInfo.withdraw = dscpInfo.dscpRmk.dscp.withdraw;
        }
        else if (0 == dscpInfo.dscpRmk.ipPrec.opcode)
        {
            actEntry_p->dscpRmkInfo.dscpRmk.ipPrec.ipPrece = dscpInfo.dscpRmk.ipPrec.ipPrece;
            actEntry_p->dscpRmkInfo.opcode = dscpInfo.dscpRmk.ipPrec.opcode;
            actEntry_p->dscpRmkInfo.withdraw = dscpInfo.dscpRmk.ipPrec.withdraw;
        }
        else if (0 == dscpInfo.dscpRmk.dtr.opcode)
        {
            actEntry_p->dscpRmkInfo.dscpRmk.dtr.dtr = dscpInfo.dscpRmk.dtr.dtr;
            actEntry_p->dscpRmkInfo.opcode = dscpInfo.dscpRmk.dtr.opcode;
            actEntry_p->dscpRmkInfo.withdraw = dscpInfo.dscpRmk.dtr.withdraw;
        }
        else
        {
            rtlglue_printf("read error dscp remarking opcode, 3\n");
        }

        infonum++;
    }

    if ((1 == asicentry.prioasn) && (infonum<3))
    {
        priInfo = *(rtl8316d_tblAsic_aclPriority_t *)&asicentry.actinfo[infonum];
        actEntry_p->prioInfo.priority = priInfo.priority;
        actEntry_p->prioInfo.withdraw = priInfo.withdraw;
    }
    
    return SUCCESS;
    
}


int32 rtl8316d_actTbl_setAsicEntry(uint32 index, rtl8316d_aclAct_entry_t * actEntry_p)
{
    rtl8316d_tblasic_aclActTbl_t asicentry;
    rtl8316d_tblAsic_aclRedirect_t redirInfo;
    rtl8316d_tblAsic_aclOtagop_t otagInfo;
    rtl8316d_tblAsic_aclIntagop_t itagInfo;
    rtl8316d_tblAsic_aclDscpRmk_t dscpInfo;
    rtl8316d_tblAsic_aclPrioRmk_t prirmkInfo;
    rtl8316d_tblAsic_aclPriority_t priInfo;
    uint32 retvalue;
    uint32 infonum = 0;

    if ((actEntry_p == NULL) || (index >= RTL8316D_ACLACT_ENTRYNUM))
        return -1;

    memset(&asicentry, 0, sizeof(asicentry));
    memset(&redirInfo, 0, sizeof(redirInfo));
    memset(&otagInfo, 0, sizeof(otagInfo));
    memset(&itagInfo, 0, sizeof(itagInfo));
    memset(&dscpInfo, 0, sizeof(dscpInfo));
    memset(&prirmkInfo, 0, sizeof(prirmkInfo));
    memset(&priInfo, 0, sizeof(priInfo));

    asicentry.fno = actEntry_p->fno;
    asicentry.copytocpu = actEntry_p->copytocpu;
    asicentry.drop = actEntry_p->drop;
    asicentry.dscprmk = actEntry_p->dscprmk;
    asicentry.itag = actEntry_p->itag;
    asicentry.mirror = actEntry_p->mirror;
    asicentry.otag = actEntry_p->otag;
    asicentry.prioasn = actEntry_p->prioasn;
    asicentry.priormk = actEntry_p->priormk;
    asicentry.redir = actEntry_p->redir;

    if (1 == asicentry.otag)
    {
        otagInfo.withdraw = actEntry_p->outTagOpInfo.withdraw;
        otagInfo.outTagOp = actEntry_p->outTagOpInfo.outTagOp;
        otagInfo.outVidCtl = actEntry_p->outTagOpInfo.outVidCtl;
        otagInfo.outVidInfo = actEntry_p->outTagOpInfo.outVidInfo;
        
        asicentry.actinfo[infonum] = *(uint32*)&otagInfo;
        infonum++;
    }
    
    if (1 == asicentry.itag)
    {
        itagInfo.withdraw = actEntry_p->inTagOpInfo.withdraw;
        itagInfo.inTagOp = actEntry_p->inTagOpInfo.inTagOp;
        itagInfo.inVidCtl = actEntry_p->inTagOpInfo.inVidCtl;
        itagInfo.inVidInfo = actEntry_p->inTagOpInfo.inVidInfo;
        
        asicentry.actinfo[infonum] = *(uint32*)&itagInfo;
        infonum ++;
    }
    
    if (1 == asicentry.priormk)
    {
        prirmkInfo.dei = actEntry_p->PriRmkInfo.dei;
        prirmkInfo.inPri = actEntry_p->PriRmkInfo.inPri;
        prirmkInfo.outPri = actEntry_p->PriRmkInfo.outPri;
        prirmkInfo.tagSel = actEntry_p->PriRmkInfo.tagSel;
        prirmkInfo.withdraw = actEntry_p->PriRmkInfo.withdraw;
        
        asicentry.actinfo[infonum] = *(uint32*)&prirmkInfo;
        infonum++;
    }

    if ((1 == asicentry.redir) && (infonum<3))
    {
        if(0 == actEntry_p->redirInfo.opcode)
        {
            redirInfo.redirect.uniRedirect.withdraw = actEntry_p->redirInfo.withdraw;
            redirInfo.redirect.uniRedirect.opcode = actEntry_p->redirInfo.opcode;
            redirInfo.redirect.uniRedirect.dpn = actEntry_p->redirInfo.redirect.uniRedirect.dpn;
        }
        else
        {
            redirInfo.redirect.multiRedirect.withdraw = actEntry_p->redirInfo.withdraw;
            redirInfo.redirect.multiRedirect.opcode = actEntry_p->redirInfo.opcode;
            redirInfo.redirect.multiRedirect.ftIdx = actEntry_p->redirInfo.redirect.multiRedirect.ftIdx;
        }
        
        asicentry.actinfo[infonum] = *(uint32*)&redirInfo;
        infonum++;
    }

    if ((1 == asicentry.dscprmk) && (infonum<3))
    {
        if (0 == actEntry_p->dscpRmkInfo.opcode)
        {
            dscpInfo.dscpRmk.dscp.withdraw = actEntry_p->dscpRmkInfo.withdraw;
            dscpInfo.dscpRmk.dscp.opcode = actEntry_p->dscpRmkInfo.opcode;
            dscpInfo.dscpRmk.dscp.acldscp = actEntry_p->dscpRmkInfo.dscpRmk.dscp.acldscp;
        }
        else if (1 == actEntry_p->dscpRmkInfo.opcode)
        {
            dscpInfo.dscpRmk.ipPrec.withdraw = actEntry_p->dscpRmkInfo.withdraw;
            dscpInfo.dscpRmk.ipPrec.opcode = actEntry_p->dscpRmkInfo.opcode;
            dscpInfo.dscpRmk.ipPrec.ipPrece = actEntry_p->dscpRmkInfo.dscpRmk.ipPrec.ipPrece;
        }
        else if (2 == actEntry_p->dscpRmkInfo.opcode)
        {
            dscpInfo.dscpRmk.dtr.withdraw = actEntry_p->dscpRmkInfo.withdraw;
            dscpInfo.dscpRmk.dtr.opcode = actEntry_p->dscpRmkInfo.opcode;
            dscpInfo.dscpRmk.dtr.dtr = actEntry_p->dscpRmkInfo.dscpRmk.dtr.dtr;
        }
        else
        {
            rtlglue_printf("dscp remarking param 'opcode' error, should not be 3\n");
        }

        asicentry.actinfo[infonum] = *(uint32 *)&dscpInfo;
        infonum++;
    }

    if ((1 == asicentry.prioasn) && (infonum<3))
    {
        priInfo.withdraw = actEntry_p->prioInfo.withdraw;
        priInfo.priority = actEntry_p->prioInfo.priority;
        asicentry.actinfo[infonum] = *(uint32*)&priInfo;
        
        infonum++;
    }

    retvalue = table_write(RTL8316D_UNIT, ACTION_TABLE, index, (uint32 *)&asicentry);

    if (retvalue != 0)
    {
        rtlglue_printf("write acl ation table entry %d failed\n", index);
        return -1;
    }
    return SUCCESS;

}

/*convert raw ASIC data to readable ACL rule entry, should fill pAclEntry->rule[n].fieldType first*/
int32 rtl8316d_inAclRule_getAsicEntry(uint32 index, rtl8316d_aclEntry_t* pAclEntry)
{
    uint32 asicContent[10];
    rtl8316d_asicPieTempl_field_t *pData, *pMask;
    uint32 retvalue;
    uint8 fieldNo;

    if (pAclEntry == NULL)
    {
        return -1;
    }
    
    if (index < 0 || index >= RTL8316D_PIE_ENTRYNUM)
    {
        rtlglue_printf("input index error %d\n", index);
        return -1;
    }

    memset(asicContent, 0, sizeof(asicContent));

    retvalue = table_read(RTL8316D_UNIT, PIE_TABLE, index, asicContent);

    if (retvalue != 0)
    {
        rtlglue_printf("read pie data table entry %d failed\n", index);
        return -1;
    }

    pData = (rtl8316d_asicPieTempl_field_t*)asicContent;
    pMask = (rtl8316d_asicPieTempl_field_t*)(asicContent + 5);
    for (fieldNo = 0; fieldNo < RTL8316D_ACLFIELD_MAX; fieldNo++)
    {
        memset(&pAclEntry->ruleField[fieldNo].mask, 0, sizeof(pAclEntry->ruleField[fieldNo].mask));
        memset(&pAclEntry->ruleField[fieldNo].data, 0, sizeof(pAclEntry->ruleField[fieldNo].data));
        switch (pAclEntry->ruleField[fieldNo].fieldType)
        {
            case PIETEM_FMT:
                pAclEntry->ruleField[fieldNo].data.un.format.noneZeroOff = pData->un.format.noneZeroOff;
                pAclEntry->ruleField[fieldNo].data.un.format.recvPort = pData->un.format.recvPort;
                pAclEntry->ruleField[fieldNo].data.un.format.tgL2Fmt = pData->un.format.tgL2Fmt;
                pAclEntry->ruleField[fieldNo].data.un.format.itagExist = pData->un.format.itagExist;
                pAclEntry->ruleField[fieldNo].data.un.format.otagExist = pData->un.format.otagExist;
                pAclEntry->ruleField[fieldNo].data.un.format.tgL23Fmt = pData->un.format.tgL23Fmt;
                pAclEntry->ruleField[fieldNo].data.un.format.tgL4Fmt = pData->un.format.tgL4Fmt;
                pAclEntry->ruleField[fieldNo].data.un.format.ispppoe = pData->un.format.ispppoe;

                pAclEntry->ruleField[fieldNo].mask.un.format.noneZeroOff = pMask->un.format.noneZeroOff;
                pAclEntry->ruleField[fieldNo].mask.un.format.recvPort = pMask->un.format.recvPort;
                pAclEntry->ruleField[fieldNo].mask.un.format.tgL2Fmt = pMask->un.format.tgL2Fmt;
                pAclEntry->ruleField[fieldNo].mask.un.format.itagExist = pMask->un.format.itagExist;
                pAclEntry->ruleField[fieldNo].mask.un.format.otagExist = pMask->un.format.otagExist;
                pAclEntry->ruleField[fieldNo].mask.un.format.tgL23Fmt = pMask->un.format.tgL23Fmt;
                pAclEntry->ruleField[fieldNo].mask.un.format.tgL4Fmt = pMask->un.format.tgL4Fmt;
                pAclEntry->ruleField[fieldNo].mask.un.format.ispppoe = pMask->un.format.ispppoe;

                break;
                
            case PIETEM_DMAC0:
                pAclEntry->ruleField[fieldNo].data.un.dmac0 = pData->un.dmac0;

                pAclEntry->ruleField[fieldNo].mask.un.dmac0 = pMask->un.dmac0;

                break;
                
            case PIETEM_DMAC1:
                pAclEntry->ruleField[fieldNo].data.un.dmac1 = pData->un.dmac1;
                pAclEntry->ruleField[fieldNo].mask.un.dmac1 = pMask->un.dmac1;
                break;
                
            case PIETEM_DMAC2:
                pAclEntry->ruleField[fieldNo].data.un.dmac2 = pData->un.dmac2;
                pAclEntry->ruleField[fieldNo].mask.un.dmac2 = pMask->un.dmac2;
                break;
                
            case PIETEM_SMAC0:
                pAclEntry->ruleField[fieldNo].data.un.smac0 = pData->un.smac0;
                pAclEntry->ruleField[fieldNo].mask.un.smac0 = pMask->un.smac0;
                break;
                
            case PIETEM_SMAC1:
                pAclEntry->ruleField[fieldNo].data.un.smac1 = pData->un.smac1;
                pAclEntry->ruleField[fieldNo].mask.un.smac1 = pMask->un.smac1;
                break;
                
            case PIETEM_SMAC2:
                pAclEntry->ruleField[fieldNo].data.un.smac2 = pData->un.smac2;
                pAclEntry->ruleField[fieldNo].mask.un.smac2 = pMask->un.smac2;
                break;
                
            case PIETEM_ETHERTYPE:
                pAclEntry->ruleField[fieldNo].data.un.ethType = pData->un.ethType;
                pAclEntry->ruleField[fieldNo].mask.un.ethType = pMask->un.ethType;
                break;
                
            case PIETEM_OTAG:
                pAclEntry->ruleField[fieldNo].data.un.otag.opri = pData->un.otag.opri;
                pAclEntry->ruleField[fieldNo].data.un.otag.dei = pData->un.otag.dei;
                pAclEntry->ruleField[fieldNo].data.un.otag.ovid = pData->un.otag.ovid;

                pAclEntry->ruleField[fieldNo].mask.un.otag.opri = pMask->un.otag.opri;
                pAclEntry->ruleField[fieldNo].mask.un.otag.dei = pMask->un.otag.dei;
                pAclEntry->ruleField[fieldNo].mask.un.otag.ovid = pMask->un.otag.ovid;

                break;
                
            case PIETEM_ITAG:
                pAclEntry->ruleField[fieldNo].data.un.itag.ipri = pData->un.itag.ipri;
                pAclEntry->ruleField[fieldNo].data.un.itag.itagExist = pData->un.itag.itagExist;
                pAclEntry->ruleField[fieldNo].data.un.itag.ivid = pData->un.itag.ivid;

                pAclEntry->ruleField[fieldNo].mask.un.itag.ipri = pMask->un.itag.ipri;
                pAclEntry->ruleField[fieldNo].mask.un.itag.itagExist = pMask->un.itag.itagExist;
                pAclEntry->ruleField[fieldNo].mask.un.itag.ivid = pMask->un.itag.ivid;

                break;
                
            case PIETEM_PRIORITY:
                pAclEntry->ruleField[fieldNo].data.un.prio.itagPri = pData->un.prio.itagPri;
                pAclEntry->ruleField[fieldNo].data.un.prio.itagExist = pData->un.prio.itagExist;
                pAclEntry->ruleField[fieldNo].data.un.prio.otagPri = pData->un.prio.otagPri;
                pAclEntry->ruleField[fieldNo].data.un.prio.dei = pData->un.prio.dei;
                pAclEntry->ruleField[fieldNo].data.un.prio.otagExist = pData->un.prio.otagExist;
                pAclEntry->ruleField[fieldNo].data.un.prio.fwdPri = pData->un.prio.fwdPri;
                pAclEntry->ruleField[fieldNo].data.un.prio.oampdu = pData->un.prio.oampdu;
                pAclEntry->ruleField[fieldNo].data.un.prio.cfi = pData->un.prio.cfi;

                pAclEntry->ruleField[fieldNo].mask.un.prio.itagPri = pMask->un.prio.itagPri;
                pAclEntry->ruleField[fieldNo].mask.un.prio.itagExist = pMask->un.prio.itagExist;
                pAclEntry->ruleField[fieldNo].mask.un.prio.otagPri = pMask->un.prio.otagPri;
                pAclEntry->ruleField[fieldNo].mask.un.prio.dei = pMask->un.prio.dei;
                pAclEntry->ruleField[fieldNo].mask.un.prio.otagExist = pMask->un.prio.otagExist;
                pAclEntry->ruleField[fieldNo].mask.un.prio.fwdPri = pMask->un.prio.fwdPri;
                pAclEntry->ruleField[fieldNo].mask.un.prio.oampdu = pMask->un.prio.oampdu;
                pAclEntry->ruleField[fieldNo].mask.un.prio.cfi = pMask->un.prio.cfi;

                break;

            case PIETEM_L2PROTOCOL:
                pAclEntry->ruleField[fieldNo].data.un.l2Proto = pData->un.l2Proto;

                pAclEntry->ruleField[fieldNo].mask.un.l2Proto = pMask->un.l2Proto;

                break;

            case PIETEM_SIP0:
                pAclEntry->ruleField[fieldNo].data.un.sip15_0 = pData->un.sip15_0;
                pAclEntry->ruleField[fieldNo].mask.un.sip15_0 = pMask->un.sip15_0;

                break;
                
            case PIETEM_SIP1:
                pAclEntry->ruleField[fieldNo].data.un.sip31_16 = pData->un.sip31_16;
                pAclEntry->ruleField[fieldNo].mask.un.sip31_16 = pMask->un.sip31_16;
                break;

            case PIETEM_DIP0:
                pAclEntry->ruleField[fieldNo].data.un.dip15_0 = pData->un.dip15_0;
                pAclEntry->ruleField[fieldNo].mask.un.dip15_0 = pMask->un.dip15_0;
                break;

            case PIETEM_DIP1:
                pAclEntry->ruleField[fieldNo].data.un.dip31_16 = pData->un.dip31_16;
                pAclEntry->ruleField[fieldNo].mask.un.dip31_16 = pMask->un.dip31_16;
                break;

            case PIETEM_IPHDR:
                pAclEntry->ruleField[fieldNo].data.un.ipHdr.dscp = pData->un.ipHdr.dscp;
                pAclEntry->ruleField[fieldNo].data.un.ipHdr.morefrag = pData->un.ipHdr.morefrag;
                pAclEntry->ruleField[fieldNo].data.un.ipHdr.noneZeroOff = pData->un.ipHdr.noneZeroOff;
                pAclEntry->ruleField[fieldNo].data.un.ipHdr.l4Proto = pData->un.ipHdr.l4Proto;

                pAclEntry->ruleField[fieldNo].mask.un.ipHdr.dscp = pMask->un.ipHdr.dscp;
                pAclEntry->ruleField[fieldNo].mask.un.ipHdr.morefrag = pMask->un.ipHdr.morefrag;
                pAclEntry->ruleField[fieldNo].mask.un.ipHdr.noneZeroOff = pMask->un.ipHdr.noneZeroOff;
                pAclEntry->ruleField[fieldNo].mask.un.ipHdr.l4Proto = pMask->un.ipHdr.l4Proto;

                break;

            case PIETEM_GIP0:
                pAclEntry->ruleField[fieldNo].data.un.grpIp15_0 = pData->un.grpIp15_0;

                pAclEntry->ruleField[fieldNo].mask.un.grpIp15_0 = pMask->un.grpIp15_0;

                break;
                
            case PIETEM_GIP1:
                pAclEntry->ruleField[fieldNo].data.un.grpIph.grpIp28_16 = pData->un.grpIph.grpIp28_16;
                pAclEntry->ruleField[fieldNo].mask.un.grpIph.grpIp28_16 = pMask->un.grpIph.grpIp28_16;

                break;

            case PIETEM_L4SPORT:
                pAclEntry->ruleField[fieldNo].data.un.l4SrcPort = pData->un.l4SrcPort;
                pAclEntry->ruleField[fieldNo].mask.un.l4SrcPort = pMask->un.l4SrcPort;

                break;

            case PIETEM_L4DPORT:
                pAclEntry->ruleField[fieldNo].data.un.l4DstPort = pData->un.l4DstPort;
                pAclEntry->ruleField[fieldNo].mask.un.l4DstPort = pMask->un.l4DstPort;
                break;

            case PIETEM_L4HDR:
                pAclEntry->ruleField[fieldNo].data.un.l4Hdr.tcpFlags = pData->un.l4Hdr.tcpFlags;
                pAclEntry->ruleField[fieldNo].data.un.l4Hdr.igmpType = pData->un.l4Hdr.igmpType;

                pAclEntry->ruleField[fieldNo].mask.un.l4Hdr.tcpFlags = pMask->un.l4Hdr.tcpFlags;
                pAclEntry->ruleField[fieldNo].mask.un.l4Hdr.igmpType = pMask->un.l4Hdr.igmpType;

                break;

            case PIETEM_ICMPCODETYPE:
                pAclEntry->ruleField[fieldNo].data.un.icmpHdr.icmpCode = pData->un.icmpHdr.icmpCode;
                pAclEntry->ruleField[fieldNo].data.un.icmpHdr.icmpType = pData->un.icmpHdr.icmpType;

                pAclEntry->ruleField[fieldNo].mask.un.icmpHdr.icmpCode = pMask->un.icmpHdr.icmpCode;
                pAclEntry->ruleField[fieldNo].mask.un.icmpHdr.icmpType = pMask->un.icmpHdr.icmpType;

                break;
                
            case PIETEM_SPM0:
                pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm0.rcvPortMask15_0 = pData->un.rcvSrcPm0.rcvPortMask15_0;

                pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm0.rcvPortMask15_0 = pMask->un.rcvSrcPm0.rcvPortMask15_0;

                break;

            case PIETEM_SPM1:
                pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.rcvPortMask24_16 = pData->un.rcvSrcPm1.rcvPortMask24_16;
                pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.dmacType = pData->un.rcvSrcPm1.dmacType;
                pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.doNotFrag = pData->un.rcvSrcPm1.doNotFrag;
                pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.ttlType = pData->un.rcvSrcPm1.ttlType;
                pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.rtkpp = pData->un.rcvSrcPm1.rtkpp;

                pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.rcvPortMask24_16 = pMask->un.rcvSrcPm1.rcvPortMask24_16;
                pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.dmacType = pMask->un.rcvSrcPm1.dmacType;
                pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.doNotFrag = pMask->un.rcvSrcPm1.doNotFrag;
                pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.ttlType = pMask->un.rcvSrcPm1.ttlType;
                pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.rtkpp = pMask->un.rcvSrcPm1.rtkpp;

                break;

            case PIETEM_RANGE0:
                pAclEntry->ruleField[fieldNo].data.un.vidRange.vidRangeChk = pData->un.vidRange.vidRangeChk;

                pAclEntry->ruleField[fieldNo].mask.un.vidRange.vidRangeChk = pMask->un.vidRange.vidRangeChk;

                break;
                
            case PIETEM_RANGE1:
                pAclEntry->ruleField[fieldNo].data.un.rangeTbl.l4Port = pData->un.rangeTbl.l4Port;
                pAclEntry->ruleField[fieldNo].data.un.rangeTbl.rcvPort = pData->un.rangeTbl.rcvPort;
                pAclEntry->ruleField[fieldNo].data.un.rangeTbl.ipRange = pData->un.rangeTbl.ipRange;

                pAclEntry->ruleField[fieldNo].mask.un.rangeTbl.l4Port = pMask->un.rangeTbl.l4Port;
                pAclEntry->ruleField[fieldNo].mask.un.rangeTbl.rcvPort = pMask->un.rangeTbl.rcvPort;
                pAclEntry->ruleField[fieldNo].mask.un.rangeTbl.ipRange = pMask->un.rangeTbl.ipRange;

                break;

            case PIETEM_PAYLOAD:
                pAclEntry->ruleField[fieldNo].data.un.payload = pData->un.payload;
                pAclEntry->ruleField[fieldNo].mask.un.payload = pMask->un.payload;

                break;

            default:                
                break;

        }
        pData++;
        pMask++;
    }

    return SUCCESS;
}


/*convert raw ASIC data to readable ACL rule entry*/
int32 rtl8316d_inAclRule_setAsicEntry(uint32 index, rtl8316d_aclEntry_t* pAclEntry)
{
    uint32 asicContent[10];
    rtl8316d_asicPieTempl_field_t *pData, *pMask;
    uint32 retvalue;
    uint8 fieldNo;

    if (pAclEntry == NULL)
    {
        return -1;
    }
    
    if (index < 0 || index >= RTL8316D_PIE_ENTRYNUM)
    {
        rtlglue_printf("input index error %d\n", index);
        return -1;
    }

    memset(asicContent, 0, sizeof(asicContent));
    pData = (rtl8316d_asicPieTempl_field_t*)asicContent;
    pMask = (rtl8316d_asicPieTempl_field_t*)(asicContent + 5);
    
    for (fieldNo = 0; fieldNo < RTL8316D_ACLFIELD_MAX; fieldNo++)
    {
        switch (pAclEntry->ruleField[fieldNo].fieldType)
        {
            case PIETEM_FMT:
                pData->un.format.noneZeroOff = pAclEntry->ruleField[fieldNo].data.un.format.noneZeroOff;
                pData->un.format.recvPort = pAclEntry->ruleField[fieldNo].data.un.format.recvPort;
                pData->un.format.tgL2Fmt = pAclEntry->ruleField[fieldNo].data.un.format.tgL2Fmt;
                pData->un.format.itagExist = pAclEntry->ruleField[fieldNo].data.un.format.itagExist;
                pData->un.format.otagExist = pAclEntry->ruleField[fieldNo].data.un.format.otagExist;
                pData->un.format.tgL23Fmt = pAclEntry->ruleField[fieldNo].data.un.format.tgL23Fmt;
                pData->un.format.tgL4Fmt = pAclEntry->ruleField[fieldNo].data.un.format.tgL4Fmt;
                pData->un.format.ispppoe = pAclEntry->ruleField[fieldNo].data.un.format.ispppoe;
                
                pMask->un.format.noneZeroOff = pAclEntry->ruleField[fieldNo].mask.un.format.noneZeroOff;
                pMask->un.format.recvPort = pAclEntry->ruleField[fieldNo].mask.un.format.recvPort;
                pMask->un.format.tgL2Fmt = pAclEntry->ruleField[fieldNo].mask.un.format.tgL2Fmt;
                pMask->un.format.itagExist = pAclEntry->ruleField[fieldNo].mask.un.format.itagExist;
                pMask->un.format.otagExist = pAclEntry->ruleField[fieldNo].mask.un.format.otagExist;
                pMask->un.format.tgL23Fmt = pAclEntry->ruleField[fieldNo].mask.un.format.tgL23Fmt;
                pMask->un.format.tgL4Fmt = pAclEntry->ruleField[fieldNo].mask.un.format.tgL4Fmt;
                pMask->un.format.ispppoe = pAclEntry->ruleField[fieldNo].mask.un.format.ispppoe;
                break;
                
            case PIETEM_DMAC0:
                pData->un.dmac0 = pAclEntry->ruleField[fieldNo].data.un.dmac0;                

                pMask->un.dmac0 = pAclEntry->ruleField[fieldNo].mask.un.dmac0;

                break;
                
            case PIETEM_DMAC1:
                
                pData->un.dmac1 = pAclEntry->ruleField[fieldNo].data.un.dmac1;

                pMask->un.dmac1 = pAclEntry->ruleField[fieldNo].mask.un.dmac1;
                break;
                
            case PIETEM_DMAC2:

                pData->un.dmac2 = pAclEntry->ruleField[fieldNo].data.un.dmac2;
                pMask->un.dmac2 = pAclEntry->ruleField[fieldNo].mask.un.dmac2 ;
                break;
                
            case PIETEM_SMAC0:
                pData->un.smac0 = pAclEntry->ruleField[fieldNo].data.un.smac0;
                pMask->un.smac0 = pAclEntry->ruleField[fieldNo].mask.un.smac0;
                break;
                
            case PIETEM_SMAC1:
                pData->un.smac1 = pAclEntry->ruleField[fieldNo].data.un.smac1;
                pMask->un.smac1 = pAclEntry->ruleField[fieldNo].mask.un.smac1;
                break;
                
            case PIETEM_SMAC2:
                pData->un.smac2 = pAclEntry->ruleField[fieldNo].data.un.smac2;

                pMask->un.smac2 = pAclEntry->ruleField[fieldNo].mask.un.smac2;
                break;
                
            case PIETEM_ETHERTYPE:
                pData->un.ethType = pAclEntry->ruleField[fieldNo].data.un.ethType;
                pMask->un.ethType = pAclEntry->ruleField[fieldNo].mask.un.ethType;
                break;
                
            case PIETEM_OTAG:
                pData->un.otag.opri = pAclEntry->ruleField[fieldNo].data.un.otag.opri;
                pData->un.otag.dei = pAclEntry->ruleField[fieldNo].data.un.otag.dei;
                pData->un.otag.ovid = pAclEntry->ruleField[fieldNo].data.un.otag.ovid;

                pMask->un.otag.opri = pAclEntry->ruleField[fieldNo].mask.un.otag.opri;
                pMask->un.otag.dei = pAclEntry->ruleField[fieldNo].mask.un.otag.dei;
                pMask->un.otag.ovid = pAclEntry->ruleField[fieldNo].mask.un.otag.ovid;

                break;
                
            case PIETEM_ITAG:
                pData->un.itag.ipri = pAclEntry->ruleField[fieldNo].data.un.itag.ipri;
                pData->un.itag.itagExist = pAclEntry->ruleField[fieldNo].data.un.itag.itagExist;
                pData->un.itag.ivid = pAclEntry->ruleField[fieldNo].data.un.itag.ivid;

                pMask->un.itag.ipri = pAclEntry->ruleField[fieldNo].mask.un.itag.ipri;
                pMask->un.itag.itagExist = pAclEntry->ruleField[fieldNo].mask.un.itag.itagExist;
                pMask->un.itag.ivid = pAclEntry->ruleField[fieldNo].mask.un.itag.ivid;

                break;
                
            case PIETEM_PRIORITY:
                pData->un.prio.itagPri = pAclEntry->ruleField[fieldNo].data.un.prio.itagPri;
                pData->un.prio.itagExist = pAclEntry->ruleField[fieldNo].data.un.prio.itagExist;
                pData->un.prio.otagPri = pAclEntry->ruleField[fieldNo].data.un.prio.otagPri;
                pData->un.prio.dei = pAclEntry->ruleField[fieldNo].data.un.prio.dei;
                pData->un.prio.otagExist = pAclEntry->ruleField[fieldNo].data.un.prio.otagExist;
                pData->un.prio.fwdPri = pAclEntry->ruleField[fieldNo].data.un.prio.fwdPri;
                pData->un.prio.oampdu = pAclEntry->ruleField[fieldNo].data.un.prio.oampdu;
                pData->un.prio.cfi = pAclEntry->ruleField[fieldNo].data.un.prio.cfi;

                pMask->un.prio.itagPri = pAclEntry->ruleField[fieldNo].mask.un.prio.itagPri;
                pMask->un.prio.itagExist = pAclEntry->ruleField[fieldNo].mask.un.prio.itagExist;
                pMask->un.prio.otagPri = pAclEntry->ruleField[fieldNo].mask.un.prio.otagPri;
                pMask->un.prio.dei = pAclEntry->ruleField[fieldNo].mask.un.prio.dei;
                pMask->un.prio.otagExist = pAclEntry->ruleField[fieldNo].mask.un.prio.otagExist;
                pMask->un.prio.fwdPri = pAclEntry->ruleField[fieldNo].mask.un.prio.fwdPri;
                pMask->un.prio.oampdu = pAclEntry->ruleField[fieldNo].mask.un.prio.oampdu;
                pMask->un.prio.cfi = pAclEntry->ruleField[fieldNo].mask.un.prio.cfi;

                break;

            case PIETEM_L2PROTOCOL:
                
                pData->un.l2Proto = pAclEntry->ruleField[fieldNo].data.un.l2Proto;
                pMask->un.l2Proto = pAclEntry->ruleField[fieldNo].mask.un.l2Proto;

                break;

            case PIETEM_SIP0:

                pData->un.sip15_0 = pAclEntry->ruleField[fieldNo].data.un.sip15_0;
                pMask->un.sip15_0 = pAclEntry->ruleField[fieldNo].mask.un.sip15_0;

                break;
                
            case PIETEM_SIP1:
                pData->un.sip31_16 = pAclEntry->ruleField[fieldNo].data.un.sip31_16;
                pMask->un.sip31_16 = pAclEntry->ruleField[fieldNo].mask.un.sip31_16;
                break;

            case PIETEM_DIP0:
                pData->un.dip15_0 = pAclEntry->ruleField[fieldNo].data.un.dip15_0;
                pMask->un.dip15_0 = pAclEntry->ruleField[fieldNo].mask.un.dip15_0;
                break;

            case PIETEM_DIP1:
                pData->un.dip31_16 = pAclEntry->ruleField[fieldNo].data.un.dip31_16;
                pMask->un.dip31_16 = pAclEntry->ruleField[fieldNo].mask.un.dip31_16;
                break;

            case PIETEM_IPHDR:
                pData->un.ipHdr.dscp = pAclEntry->ruleField[fieldNo].data.un.ipHdr.dscp;
                pData->un.ipHdr.morefrag = pAclEntry->ruleField[fieldNo].data.un.ipHdr.morefrag;
                pData->un.ipHdr.noneZeroOff = pAclEntry->ruleField[fieldNo].data.un.ipHdr.noneZeroOff;
                pData->un.ipHdr.l4Proto = pAclEntry->ruleField[fieldNo].data.un.ipHdr.l4Proto;

                pMask->un.ipHdr.dscp = pAclEntry->ruleField[fieldNo].mask.un.ipHdr.dscp;
                pMask->un.ipHdr.morefrag = pAclEntry->ruleField[fieldNo].mask.un.ipHdr.morefrag;
                pMask->un.ipHdr.noneZeroOff = pAclEntry->ruleField[fieldNo].mask.un.ipHdr.noneZeroOff;
                pMask->un.ipHdr.l4Proto = pAclEntry->ruleField[fieldNo].mask.un.ipHdr.l4Proto;

                break;

            case PIETEM_GIP0:
                pData->un.grpIp15_0 = pAclEntry->ruleField[fieldNo].data.un.grpIp15_0;
                pMask->un.grpIp15_0 = pAclEntry->ruleField[fieldNo].mask.un.grpIp15_0;

                break;
                
            case PIETEM_GIP1:
                pData->un.grpIph.grpIp28_16 = pAclEntry->ruleField[fieldNo].data.un.grpIph.grpIp28_16;
                pMask->un.grpIph.grpIp28_16 = pAclEntry->ruleField[fieldNo].mask.un.grpIph.grpIp28_16;

                break;

            case PIETEM_L4SPORT:
                pData->un.l4SrcPort = pAclEntry->ruleField[fieldNo].data.un.l4SrcPort;
                pMask->un.l4SrcPort = pAclEntry->ruleField[fieldNo].mask.un.l4SrcPort;

                break;

            case PIETEM_L4DPORT:
                pData->un.l4DstPort = pAclEntry->ruleField[fieldNo].data.un.l4DstPort ;
                pMask->un.l4DstPort = pAclEntry->ruleField[fieldNo].mask.un.l4DstPort;
                break;

            case PIETEM_L4HDR:
                pData->un.l4Hdr.tcpFlags = pAclEntry->ruleField[fieldNo].data.un.l4Hdr.tcpFlags;                
                pData->un.l4Hdr.igmpType = pAclEntry->ruleField[fieldNo].data.un.l4Hdr.igmpType;

                pMask->un.l4Hdr.tcpFlags = pAclEntry->ruleField[fieldNo].mask.un.l4Hdr.tcpFlags;
                pMask->un.l4Hdr.igmpType = pAclEntry->ruleField[fieldNo].mask.un.l4Hdr.igmpType;

                break;

            case PIETEM_ICMPCODETYPE:
                pData->un.icmpHdr.icmpCode = pAclEntry->ruleField[fieldNo].data.un.icmpHdr.icmpCode;
                pData->un.icmpHdr.icmpType = pAclEntry->ruleField[fieldNo].data.un.icmpHdr.icmpType;

                pMask->un.icmpHdr.icmpCode = pAclEntry->ruleField[fieldNo].mask.un.icmpHdr.icmpCode;
                pMask->un.icmpHdr.icmpType = pAclEntry->ruleField[fieldNo].mask.un.icmpHdr.icmpType;

                break;
                
            case PIETEM_SPM0:
                pData->un.rcvSrcPm0.rcvPortMask15_0 = pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm0.rcvPortMask15_0;
                pMask->un.rcvSrcPm0.rcvPortMask15_0 = pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm0.rcvPortMask15_0;

                break;

            case PIETEM_SPM1:
                pData->un.rcvSrcPm1.rcvPortMask24_16 = pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.rcvPortMask24_16;
                pData->un.rcvSrcPm1.dmacType = pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.dmacType;
                pData->un.rcvSrcPm1.doNotFrag = pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.doNotFrag;
                pData->un.rcvSrcPm1.ttlType = pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.ttlType;
                pData->un.rcvSrcPm1.rtkpp = pAclEntry->ruleField[fieldNo].data.un.rcvSrcPm1.rtkpp;

                pMask->un.rcvSrcPm1.rcvPortMask24_16 = pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.rcvPortMask24_16;
                pMask->un.rcvSrcPm1.dmacType = pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.dmacType;
                pMask->un.rcvSrcPm1.doNotFrag = pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.doNotFrag;
                pMask->un.rcvSrcPm1.ttlType = pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.ttlType;
                pMask->un.rcvSrcPm1.rtkpp = pAclEntry->ruleField[fieldNo].mask.un.rcvSrcPm1.rtkpp;

                break;

            case PIETEM_RANGE0:
                pData->un.vidRange.vidRangeChk = pAclEntry->ruleField[fieldNo].data.un.vidRange.vidRangeChk ;
                pMask->un.vidRange.vidRangeChk = pAclEntry->ruleField[fieldNo].mask.un.vidRange.vidRangeChk;

                break;
                
            case PIETEM_RANGE1:
                pData->un.rangeTbl.l4Port = pAclEntry->ruleField[fieldNo].data.un.rangeTbl.l4Port;
                pData->un.rangeTbl.rcvPort = pAclEntry->ruleField[fieldNo].data.un.rangeTbl.rcvPort;
                pData->un.rangeTbl.ipRange = pAclEntry->ruleField[fieldNo].data.un.rangeTbl.ipRange;

                pMask->un.rangeTbl.l4Port= pAclEntry->ruleField[fieldNo].mask.un.rangeTbl.l4Port;
                pMask->un.rangeTbl.rcvPort= pAclEntry->ruleField[fieldNo].mask.un.rangeTbl.rcvPort;
                pMask->un.rangeTbl.ipRange = pAclEntry->ruleField[fieldNo].mask.un.rangeTbl.ipRange ;

                break;

            case PIETEM_PAYLOAD:
                pData->un.payload = pAclEntry->ruleField[fieldNo].data.un.payload;
                pMask->un.payload = pAclEntry->ruleField[fieldNo].mask.un.payload;

                break;

            default:                
                break;

        }
        pData++;
        pMask++;
    }

    retvalue = table_write(RTL8316D_UNIT, PIE_TABLE, index, asicContent);
    
    if (retvalue != 0)
    {
        rtlglue_printf("write pie data table entry %d failed\n", index);
        return -1;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtk_filter_templ_set
 * Description:
 *      Set template field type
 * Input:
 *      blockId  - ACL block ID
 *      fieldId  - template field index
 *      fieldType  - template type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set template field type  
 */
rtk_api_ret_t rtk_filter_templ_set(rtk_aclBlock_id_t blockId, rtk_filter_templField_id_t fieldId, rtk_filter_field_type_t fieldType)
{
    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 regVal;

    if (blockId >= RTK_ACL_BLOCK_MAX || fieldId >= RTK_ACL_TEMPLFIELD_MAX || fieldType >= PIETEM_END)
    {
        return RT_ERR_INPUT;
    }

    regAddr = USER_DEFINED_TEMPLATE_CONTROL0 + blockId*2 + fieldId/6;
    reg_read(RTL8316D_UNIT, regAddr, &regVal);
    regVal &= ~(0x1F << ((fieldId%6) * 5));
    regVal |= fieldType << ((fieldId%6) * 5);
    retVal = reg_write(RTL8316D_UNIT, regAddr, regVal);

    return retVal;    
}

/* Function Name:
 *      rtk_filter_templ_get
 * Description:
 *      Get template field type
 * Input:
 *      blockId  - ACL block ID
 *      fieldId  - template field index
 * Output:
 *      pFieldType - template type
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get template field type  
 */
rtk_api_ret_t rtk_filter_templ_get(rtk_aclBlock_id_t blockId, rtk_filter_templField_id_t fieldId, rtk_filter_field_type_t* pFieldType)
{
    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 regVal;

    if (blockId >= RTK_ACL_BLOCK_MAX || fieldId >= RTK_ACL_TEMPLFIELD_MAX )
    {
        return RT_ERR_INPUT;
    }

    if (pFieldType == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    regAddr = USER_DEFINED_TEMPLATE_CONTROL0 + blockId*2 + fieldId/6;
    retVal = reg_read(RTL8316D_UNIT, regAddr, &regVal);
    *pFieldType = (regVal >> ((fieldId%6) * 5)) & 0x1F;

    return retVal;    
}

/* Function Name:
 *      rtk_filter_reverseHit_set
 * Description:
 *      Reverse the ACL rule hit result
 * Input:
 *      blockId  - ACL block ID
 *      ruleId  - rule specified
 *      reverse  - 1: reverse, 0: don't reverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid input parameters.
 * Note:
 *      This API can reverse the hit result of specified ACL rule.
 */
rtk_api_ret_t rtk_filter_reverseHit_set(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t reverse)
{
    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 regVal;

    if (blockId >= RTK_ACL_BLOCK_MAX || ruleId >= RTL8316D_PIELOGIC_BLKENTRY)
    {
        return RT_ERR_INPUT;
    }
    if (reverse >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    regAddr = PIE_RESULT_REVERSE_CONTROL0 + blockId;
    reg_read(RTL8316D_UNIT, regAddr, &regVal);
    regVal &= ~(1 << ruleId);
    regVal |= 1 << ruleId;
    retVal = reg_write(RTL8316D_UNIT, regAddr, regVal);

    return retVal;    
}

/* Function Name:
 *      rtk_filter_reverseHit_get
 * Description:
 *      Retrieve  reverse status of ACL rule hit result
 * Input:
 *      blockId  - ACL block ID
 *      ruleId  - rule specified
 * Output:
 *      pReverse  - 1: reverse, 0: don't reverse
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve the reverse status of the specified ACL rule
 */
rtk_api_ret_t rtk_filter_reverseHit_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t* pReverse)
{
    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 regVal;

    if (blockId >= RTK_ACL_BLOCK_MAX || ruleId >= RTL8316D_PIELOGIC_BLKENTRY)
    {
        return RT_ERR_INPUT;
    }

    if(pReverse == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    regAddr = PIE_RESULT_REVERSE_CONTROL0 + blockId;
    retVal = reg_read(RTL8316D_UNIT, regAddr, &regVal);
    *pReverse = (regVal >> ruleId) & 0x1;

    return retVal;    
}

/* Function Name:
 *      rtk_filter_aclEntry_alloc
 * Description:
 *      alloc an ACL entry
 * Input:
 *      None
 * Output:
 *      blockId  - pointer to ACL entry address returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can alloc an ACL entry. 
 */
rtk_api_ret_t rtk_filter_aclEntry_alloc(rtk_filter_aclEntry_t** ppAclEntry)
{
    rtl8316d_aclAct_entry_t* pAction;

    if (ppAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    *ppAclEntry = (rtk_filter_aclEntry_t*) malloc(sizeof(rtk_filter_aclEntry_t));
    if (*ppAclEntry == NULL)
    {
        return RT_ERR_FAILED; /*insufficient memory*/
    }
    memset(*ppAclEntry, 0, sizeof(rtk_filter_aclEntry_t));
    (*ppAclEntry)->next = (*ppAclEntry)->prev = NULL;
    
    pAction = (rtl8316d_aclAct_entry_t*)malloc(sizeof(rtl8316d_aclAct_entry_t));
    if (pAction == NULL)
    {
        free(*ppAclEntry);
        return RT_ERR_FAILED; /*insufficient memory*/
    }

    (*ppAclEntry)->pAction = (uint32*)pAction;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_l2CrcErrProc_set
 * Description:
 *      Enable/Disable ACL lookup on packets which have Layer2 CRC error.
 * Input:
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API enable/disable ACL lookup on packets which have Layer2 CRC error.
 *       (1). ENABLED means ACL will process packets with CRC error.
 *       (2). DISABLE means ACL will skip packets which have CRC error.
 */
rtk_api_ret_t rtk_filter_l2CrcErrProc_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(enable > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = reg_field_write(RTL8316D_UNIT, CRC_ERROR_FRAME_LOOKUP_CONTROL, CRCERRPIELK, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_filter_l2CrcErrProc_get
 * Description:
 *      Retrieve whether ACL lookup applies on packets which have Layer2 CRC error or not.
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve whether ACL lookup applies on packets which have Layer2 CRC error or not.
 *       (1). ENABLED means ACL will process packets with CRC error.
 *       (2). DISABLE means ACL will skip packets which have CRC error.
 */
rtk_api_ret_t rtk_filter_l2CrcErrProc_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if(pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_field_read(RTL8316D_UNIT, CRC_ERROR_FRAME_LOOKUP_CONTROL, CRCERRPIELK, pEnable);
    
    return retVal;
}

/* Function Name:
 *      rtk_filter_cfiSetProc_set
 * Description:
 *      Enable/Disable ACL lookup on packets which are tagged and have CFI bit set.
 * Input:
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API enable/disable ACL lookup on packets which are tagged and have CFI bit set.
 *       (1). ENABLED means ACL will process packets with CFI set.
 *       (2). DISABLE means ACL will skip packets which have CFI set.
 */
rtk_api_ret_t rtk_filter_cfiSetProc_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(enable > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = reg_field_write(RTL8316D_UNIT, CRC_ERROR_FRAME_LOOKUP_CONTROL, CFIPIELK, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_filter_cfiSetProc_get
 * Description:
 *      Retrieve whether ACL lookup applies on packets which are tagged and have CFI set.
 * Input:
 *      None
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve whether ACL processes packets which are tagged and have CFI bit set or not.
 *       (1). ENABLED means ACL will process packets tagged with CFI set.
 *       (2). DISABLE means ACL will skip packets which are tagged and have CFI set.
 */
rtk_api_ret_t rtk_filter_cfiSetProc_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if(pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_field_read(RTL8316D_UNIT, CRC_ERROR_FRAME_LOOKUP_CONTROL, CFIPIELK, pEnable);
    
    return retVal;
}


/* Function Name:
 *      rtk_filter_portEnable_set
 * Description:
 *      Enable/Disable ACL lookup on a per-port basis.
 * Input:
 *      port - port Id
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port ID.
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API enable/disable ACL lookup on a per-port basis.
 */
rtk_api_ret_t rtk_filter_portEnable_set(rtk_filter_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if(enable > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = reg_field_write(RTL8316D_UNIT, PORT0_ACL_LOOKUP_CONTROL+port, INACLLUEN, enable);
    
    return retVal;
}

/* Function Name:
 *      rtk_filter_portEnable_get
 * Description:
 *      Retrieve ACL lookup ability on a per-port basis.
 * Input:
 *      port - port Id
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API retrieve ACL lookup ability on a per-port basis..
 */
rtk_api_ret_t rtk_filter_portEnable_get(rtk_filter_port_t port, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if(pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_field_read(RTL8316D_UNIT, PORT0_ACL_LOOKUP_CONTROL+port, INACLLUEN, pEnable);
    
    return retVal;
}


/* Function Name:
 *      rtk_filter_aclEntry_init
 * Description:
 *      Initialize ACL entry
 * Input:
 *      pAclEntry - ACL entry specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can initialize an ACL entry  
 */
rtk_api_ret_t rtk_filter_aclEntry_init(rtk_filter_aclEntry_t* pAclEntry)
{
    rtl8316d_aclAct_entry_t* pActTmp;

    if (pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    pAclEntry->ruleId = RTL8316D_ACLRULEID_AUTO;
    pAclEntry->fieldNum = 0;
    pActTmp = (rtl8316d_aclAct_entry_t*)pAclEntry->pAction;
    memset(pActTmp, 0, sizeof(rtl8316d_aclAct_entry_t));

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_filter_aclEntry_free
 * Description:
 *      Release resource occupied by ACL entry
 * Input:
 *      pAclEntry - ACL entry specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can destroy an ACL entry, and release all resource it occupies.  
 */
rtk_api_ret_t rtk_filter_aclEntry_free(rtk_filter_aclEntry_t* pAclEntry)
{
    rtk_filter_aclField_t* pField, *pCur;
    rtl8316d_aclAct_entry_t* pAction;

    pAction = (rtl8316d_aclAct_entry_t*)pAclEntry->pAction;
    free(pAction);
    pAclEntry->pAction = NULL;
    
    for (pField = pAclEntry->pField; pField != NULL; )
    {
        pAclEntry->fieldNum--;
        pCur = pField;
        pField = pField->next;
        free(pCur);
    }

    free(pAclEntry);
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_aclField_add
 * Description:
 *       Add field to ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 *      fieldData - field data
 *      fieldMask - field Mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER -
 * Note:
 *      The API can add particular field to specified ACL rule.  
 */
rtk_api_ret_t rtk_filter_aclField_add(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_pieTempl_field_t fieldData, rtk_filter_pieTempl_field_t fieldMask)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclField_t* pCur;
    rtk_filter_aclField_t* pFieldNode;

    if (pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    if (type >= PIETEM_END)
    {
        return RT_ERR_INPUT;
    }

    if (pAclEntry->fieldNum >= RTL8316D_ACLFIELD_MAX)
    {
        return RT_ERR_FAILED; 
    }

    if ((pFieldNode = (rtk_filter_aclField_t*)malloc(sizeof(rtk_filter_aclField_t))) == NULL)
    {
        return RT_ERR_FAILED; /*insufficient memory*/
    }
    memset(pFieldNode, 0, sizeof(rtk_filter_aclField_t));
    pFieldNode->type = type;
    pFieldNode->next = NULL;
    memcpy(&pFieldNode->fieldData, &fieldData, sizeof(rtk_filter_pieTempl_field_t));
    memcpy(&pFieldNode->fieldMask, &fieldMask, sizeof(rtk_filter_pieTempl_field_t));            

    /*ACL rule is empty*/
    if (pAclEntry->pField == NULL)
    {
        pAclEntry->pField = pFieldNode;
        pAclEntry->fieldNum++;
        return RT_ERR_OK;
    }

    /*non-empty ACL rule*/
    if (pAclEntry->pField->type == type)
    {   
        goto ERRPROC; /*type exists already*/
    } else if (pAclEntry->pField->type > type)
    {
        pFieldNode->next = pAclEntry->pField;
        pAclEntry->pField = pFieldNode;
        pAclEntry->fieldNum++;
        return RT_ERR_OK;
    }

    for (pCur = pAclEntry->pField; pCur->next != NULL; pCur = pCur->next)
    {
        if (pCur->next->type == type)
        {
             goto ERRPROC; /*type exists already*/
        } else if (pCur->next->type > type)
        {
            break;
        } 
    }

    /*append*/
    if (pCur->next == NULL)
    {
        pCur->next = pFieldNode;
    } else { /*insert*/
        pFieldNode->next = pCur->next;
        pCur->next = pFieldNode;
    }

    pAclEntry->fieldNum++;
    return RT_ERR_OK;  

ERRPROC:
    free(pFieldNode);
    pFieldNode = NULL;
    return RT_ERR_FAILED;  
}

/* Function Name:
 *      rtk_filter_aclField_del
 * Description:
 *       Delete field from ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER -
 * Note:
 *      The API can remove particular field from specified ACL rule. 
 */
rtk_api_ret_t rtk_filter_aclField_del(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclField_t* pCur, *pNext;

    if (pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    if (type >= PIETEM_END)
    {
        return RT_ERR_INPUT;
    }

    /*ACL rule is empty*/
    if (pAclEntry->pField == NULL)
    {
        return RT_ERR_FILTER_INACL_EMPTY; /*field not found*/
    }

    pCur = pAclEntry->pField;    
    if (pCur->type == type)
    {    
        pAclEntry->fieldNum--;
        pAclEntry->pField = pCur->next;
        free(pCur);
        pCur = NULL;
        return RT_ERR_OK;
    }

    for (pNext = pCur->next; pNext != NULL; pNext = pNext->next)
    {
        if (pNext->type == type)
        {
            pAclEntry->fieldNum--;
            pCur->next = pNext->next;
            free(pNext);
            pNext = NULL;
            return RT_ERR_OK;
        } else if (pNext->type > type)
        {
            return RT_ERR_FILTER_RULEFIELD_NOTFOUND; /*type not found*/
        }
        
        pCur = pNext;
    }

    return RT_ERR_FILTER_RULEFIELD_NOTFOUND; /*field not found*/
}

/* Function Name:
 *      rtk_filter_aclField_getByType
 * Description:
 *       Get specified type field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 *      pField -  field returned
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve particular type field from ACL rule.  
 */
rtk_api_ret_t rtk_filter_aclField_getByType(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_aclField_t* pField)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclField_t* pCur;

    if (pAclEntry == NULL || pField == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    if (type >= PIETEM_END)
    {
        return RT_ERR_INPUT;
    }
    
    /*ACL rule is empty*/
    if (pAclEntry->pField == NULL)
    {
        return RT_ERR_FILTER_INACL_EMPTY;
    }

    for (pCur = pAclEntry->pField; pCur != NULL; pCur = pCur->next)
    {
        if (pCur->type == type)
        {
            pField->type = type;
            memcpy(&pField->fieldData, &pCur->fieldData, sizeof(rtk_filter_pieTempl_field_t));
            memcpy(&pField->fieldMask, &pCur->fieldMask, sizeof(rtk_filter_pieTempl_field_t));
            return RT_ERR_OK;
        } 
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_filter_aclField_getFirst
 * Description:
 *      Get first field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      pField -  field returned
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve first field from ACL rule.    
 */
rtk_api_ret_t rtk_filter_aclField_getFirst(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_aclField_t* pField)
{
//    rtk_api_ret_t retVal;

    if (pField == NULL || pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
        
    /*ACL rule is empty*/
    if (pAclEntry->pField == NULL)
    {
        return RT_ERR_FILTER_INACL_EMPTY;
    } else {
        pField->type = pAclEntry->pField->type;
        memcpy(&pField->fieldData, &pAclEntry->pField->fieldData, sizeof(rtk_filter_pieTempl_field_t));
        memcpy(&pField->fieldMask, &pAclEntry->pField->fieldMask, sizeof(rtk_filter_pieTempl_field_t));
        return RT_ERR_OK; /*type exists*/
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_filter_aclField_getNext
 * Description:
 *      Get next field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      type -  field type
 * Output:
 *      pField - field information returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can retrieve next field which type is larger than that specified in argument type from ACL rule. 
 */
rtk_api_ret_t rtk_filter_aclField_getNext(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_aclField_t* pField)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclField_t* pCur; //, *pNext;

    if (pAclEntry == NULL || pField == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    if (type >= PIETEM_END)
    {
        return RT_ERR_FILTER_RULEFIELD_NO;
    }
        
    /*ACL rule is empty*/
    if (pAclEntry->pField == NULL)
    {
        return RT_ERR_FILTER_INACL_EMPTY;
    } 
    
    for (pCur = pAclEntry->pField; pCur != NULL; pCur= pCur->next)
    {
        if (pCur->type > type)
        {
            pField->type = pCur->type;
            memcpy(&pField->fieldData, &pCur->fieldData, sizeof(rtk_filter_pieTempl_field_t));
            memcpy(&pField->fieldMask, &pCur->fieldMask, sizeof(rtk_filter_pieTempl_field_t));            
            return RT_ERR_OK;
        }            
    }   

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_filter_aclField_replace
 * Description:
 *      Replace specified type field of ACL rule
 * Input:
 *      pAclEntry - ACL rule
 *      pField - new field specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *      The API can replace particular type field of ACL rule.   
 */
rtk_api_ret_t rtk_filter_aclField_replace(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_aclField_t* pField)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclField_t* pCur;

    if (pAclEntry == NULL || pField == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    if (pField->type >= PIETEM_END)
    {
        return RT_ERR_INPUT;
    }
    
    /*ACL rule is empty*/
    if (pAclEntry->pField == NULL)
    {
        return RT_ERR_FILTER_INACL_EMPTY;
    }

    for (pCur = pAclEntry->pField; pCur != NULL; pCur = pCur->next)
    {
        if (pCur->type == pField->type)
        {
            memcpy(&pCur->fieldData, &pField->fieldData, sizeof(rtk_filter_pieTempl_field_t));
            memcpy(&pCur->fieldMask, &pField->fieldMask, sizeof(rtk_filter_pieTempl_field_t));
            return RT_ERR_OK;
        } 
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_filter_aclRule_append
 * Description:
 *      Append ACL
 * Input:
 *      pAclEntry - ACL rule
 *      blockId - ACL block ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - Insufficient space.
 * Note:
 *      The API can replace particular type field of ACL rule.   
 */
rtk_api_ret_t rtk_filter_aclRule_append(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId)
{
//    rtk_api_ret_t retVal;
//    rtk_filter_aclEntry_t* pCur;

    if (pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (blockId >= RTK_ACL_BLOCK_MAX)
    {
        return RT_ERR_FILTER_BLOCKNUM;
    }

    if ((pAclGroup[blockId].entryNum != 0) && (pAclGroup[blockId].aclEntry->prev->ruleId >= (RTL8316D_PIELOGIC_BLKENTRY - 1)))
    {
        return RT_ERR_FILTER_INACL_RULENUM; /*insufficient space*/
    }

    if (pAclEntry->ruleId != RTL8316D_ACLRULEID_AUTO)
    {
        return RT_ERR_FILTER_ENTRYIDX;
    } 

    if (pAclGroup[blockId].entryNum == 0)
    {
        pAclGroup[blockId].aclEntry = pAclEntry;
        pAclEntry->next = pAclEntry->prev = pAclEntry;
        pAclEntry->ruleId = 0;
    } else {
        pAclEntry->next = pAclGroup[blockId].aclEntry;
        pAclEntry->prev = pAclGroup[blockId].aclEntry->prev;
        pAclGroup[blockId].aclEntry->prev->next = pAclEntry;
        pAclGroup[blockId].aclEntry->prev = pAclEntry;
        pAclEntry->ruleId = pAclEntry->prev->ruleId + 1;
    }

    pAclGroup[blockId].entryNum++;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_aclRule_insert
 * Description:
 *      Insert ACL
 * Input:
 *      pAclEntry - ACL rule
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - No room available for more ACL rules.
 * Note:
 *      The API can insert ACL rule to the position specified in pAclEntry->ruleId.
 */
rtk_api_ret_t rtk_filter_aclRule_insert(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclEntry_t* pCur;

    if (pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (blockId >= RTK_ACL_BLOCK_MAX)
    {
        return RT_ERR_FILTER_BLOCKNUM;
    }
    
    if (ruleId >= RTL8316D_PIELOGIC_BLKENTRY)
    {
        return RT_ERR_FILTER_INACL_RULENUM; /*insufficient space*/
    }

    if (pAclGroup[blockId].entryNum >= RTL8316D_PIELOGIC_BLKENTRY)
    {
        return RT_ERR_FILTER_INACL_RULENUM; /*insufficient space*/
    }

    if (pAclGroup[blockId].entryNum == 0)
    {
        pAclGroup[blockId].aclEntry = pAclEntry;
        pAclEntry->next = pAclEntry->prev = pAclEntry;
        pAclGroup[blockId].entryNum++;    
        pAclEntry->ruleId = ruleId;
        return RT_ERR_OK;
    } else {
        pCur = pAclGroup[blockId].aclEntry;
        do{
            if (pCur->ruleId == ruleId)
            {   /*entry exists already*/
                return RT_ERR_FILTER_INACL_RULENUM;
            } else if (pCur->ruleId > ruleId)
            {
                break;
            }
            
            pCur = pCur->next;
        }while(pCur != pAclGroup[blockId].aclEntry);

        /*insert entry*/
        pAclEntry->next = pCur;
        pAclEntry->prev = pCur->prev;
        pCur->prev->next = pAclEntry;
        pCur->prev = pAclEntry;
        pAclEntry->ruleId = ruleId;
        pAclGroup[blockId].entryNum++;   
        if (pAclGroup[blockId].aclEntry == pCur && pCur->ruleId > ruleId)
        {   /*insert infront of the list head*/
            pAclGroup[blockId].aclEntry = pAclEntry;
        }
        return RT_ERR_OK;

    }

}

/* Function Name:
 *      rtk_filter_aclRule_insert
 * Description:
 *      Insert ACL
 * Input:
 *      pAclEntry - ACL rule
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - No room available for more ACL rules.
 * Note:
 *      The API can remove ACL rule from ACL list, the rule removed should be released
 *      by calling rtk_filter_aclEntry_free( ) .   
 */
rtk_api_ret_t rtk_filter_aclRule_remove(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_filter_aclEntry_t** ppAclEntry)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclEntry_t* pCur;
    uint8 found;

    if (blockId >= RTK_ACL_BLOCK_MAX)
    {
        return RT_ERR_FILTER_BLOCKNUM;
    }
    if (ruleId >= RTL8316D_PIELOGIC_BLKENTRY)
    {
        return RT_ERR_FILTER_INACL_RULENUM;
    }

    if (ppAclEntry == NULL && *ppAclEntry ==  NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    if (pAclGroup[blockId].entryNum == 0)
    {
        return RT_ERR_FILTER_INACL_RULENUM; /*entry not found*/
    }

    if (pAclGroup[blockId].entryNum == 1 && pAclGroup[blockId].aclEntry->ruleId == ruleId)
    {
        *ppAclEntry = pAclGroup[blockId].aclEntry;
        pAclGroup[blockId].aclEntry = NULL;
        pAclGroup[blockId].entryNum--;
        return RT_ERR_OK;
    }
    
    /*more than one entry*/
    found = 0;
    pCur = pAclGroup[blockId].aclEntry;
    do{
        if (pCur->ruleId == ruleId)
        {
            found = 1;
            break;
        } 
        pCur = pCur->next;
    } while(pCur != pAclGroup[blockId].aclEntry);

    if (found == 1){
        pAclGroup[blockId].entryNum--;
        if (pAclGroup[blockId].aclEntry == pCur)
        {
            pAclGroup[blockId].aclEntry = pCur->next;
        }        
        pCur->prev->next = pCur->next;
        pCur->next->prev = pCur->prev;
       *ppAclEntry = pCur;
        return RT_ERR_OK;
    }
    
    return RT_ERR_FILTER_INACL_RULENUM; /*entry not found*/
}

/* Function Name:
 *      rtk_filter_aclRule_get
 * Description:
 *      Get ACL rule
 * Input:
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      ppAclEntry - Pointer to pointer which refers to the address of ACL rule retrieved.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_INACL_RULENUM - No room available for more ACL rules.
 * Note:
 *      The API can get ACL rule from ACL list.     
 */
rtk_api_ret_t rtk_filter_aclRule_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_filter_aclEntry_t** ppAclEntry)
{
//    rtk_api_ret_t retVal;
    rtk_filter_aclEntry_t* pCur;
    uint8 found;

    if (ppAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    if (blockId >= RTK_ACL_BLOCK_MAX)
    {
        return RT_ERR_FILTER_BLOCKNUM;
    }
    if (ruleId >= RTL8316D_PIELOGIC_BLKENTRY)
    {
        return RT_ERR_FILTER_INACL_RULENUM;
    }
    
    if (pAclGroup[blockId].entryNum == 0)
    {
        return RT_ERR_FILTER_INACL_RULENUM; /*entry not found*/
    }

    found = 0;
    pCur = pAclGroup[blockId].aclEntry;
    do{
        if (pCur->ruleId == ruleId)
        {
            found = 1;
            break;
        } 
        pCur = pCur->next;
    } while(pCur != pAclGroup[blockId].aclEntry);

    if (found == 1){        
        *ppAclEntry = pCur; 
        return RT_ERR_OK;
    }
    
    return RT_ERR_FAILED; /*entry not found*/
}

/* Function Name:
 *      rtk_filter_aclRule_getFirst
 * Description:
 *      Get ACL first rule
 * Input:
 *      blockId - ACL block ID
 * Output:
 *      ppAclEntry - Pointer to pointer which refers to the address of ACL rule retrieved.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 * Note:
 *      The API can get ACL rule from ACL list.     
 */
rtk_api_ret_t rtk_filter_aclRule_getFirst(rtk_aclBlock_id_t blockId, rtk_filter_aclEntry_t** ppAclEntry)
{
    if (ppAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    if (blockId >= RTK_ACL_BLOCK_MAX)
    {
        return RT_ERR_FILTER_BLOCKNUM;
    }
    
    if (pAclGroup[blockId].entryNum == 0)
    {
        return RT_ERR_FAILED; /*entry not found*/
    }

    *ppAclEntry = pAclGroup[blockId].aclEntry;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_aclRule_write
 * Description:
 *      Write ACL rule into ASIC
 * Input:
 *      pAclEntry - ACL rule to be written
 *      blockId - ACL block ID
 *      ruleId - ACL rule ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_BLOCKNUM - Invalid block ID
 *      RT_ERR_FILTER_ENTRYIDX - Invalid ACL rule index.
 * Note:
 *       The API can write ACL rule into ASIC.       
 */
rtk_api_ret_t rtk_filter_aclRule_write(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId)
{
//    rtk_api_ret_t retVal;

    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_aclAction_set
 * Description:
 *      Add specified type of ACL action
 * Input:
 *      pAclEntry - ACL rule to be written
 *      actType - action type
 *      pAction - ACL action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_ACTION - Invalid action.
 * Note:
 *       The API can add ACL action.      
 */
rtk_api_ret_t rtk_filter_aclAction_set(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType, rtk_filter_aclAction_t* pAction)
{
    rtl8316d_aclAct_entry_t* pActTmp;
    
    if (pAclEntry == NULL || pAction == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (actType >= ACLACTTYPE_END)
    {
        return RT_ERR_FILTER_ACTION;
    }

    pActTmp = (rtl8316d_aclAct_entry_t*)pAclEntry->pAction;
    
    switch(actType)
    {
        case ACLACTTYPE_FNO: 
             pActTmp->fno = pAction->un.fno;
             break;
        
        case ACLACTTYPE_DROP:
            pActTmp->drop = pAction->un.dropInfo;
             break;
             
        case ACLACTTYPE_CP2CPU: 
            pActTmp->copytocpu = pAction->un.cp2cpu;
             break;
         
        case ACLACTTYPE_MIRROR:
            pActTmp->mirror = pAction->un.mirror;
             break;
             
        case ACLACTTYPE_OTAG:
            if (pActTmp->otag == 0 && pActTmp->actNum >= RTL8316D_ACLACTINFO_LENGTH)
            {
                return RT_ERR_FILTER_ACTION_NUMBER;
            } else if (pActTmp->otag == 0){
                pActTmp->actNum++;
                pActTmp->otag = 1;               
            }

            pActTmp->outTagOpInfo.withdraw = pAction->un.outTagOpInfo.withdraw;
            pActTmp->outTagOpInfo.outVidCtl= pAction->un.outTagOpInfo.outVidCtl;
            pActTmp->outTagOpInfo.outVidInfo = pAction->un.outTagOpInfo.outVidInfo;
            pActTmp->outTagOpInfo.outTagOp = pAction->un.outTagOpInfo.outTagOp;
             break;
         
        case ACLACTTYPE_ITAG:
            if (pActTmp->itag == 0 && pActTmp->actNum >= RTL8316D_ACLACTINFO_LENGTH)
            {
                return RT_ERR_FILTER_ACTION_NUMBER;
            } else if (pActTmp->itag == 0){
                pActTmp->actNum++;
                pActTmp->itag = 1;               
            }

             pActTmp->inTagOpInfo.withdraw = pAction->un.inTagOpInfo.withdraw;
             pActTmp->inTagOpInfo.inVidCtl= pAction->un.inTagOpInfo.inVidCtl;
             pActTmp->inTagOpInfo.inVidInfo = pAction->un.inTagOpInfo.inVidInfo;
             pActTmp->inTagOpInfo.inTagOp = pAction->un.inTagOpInfo.inTagOp;
             break;
             
        case ACLACTTYPE_PRIRMK: 
            if (pActTmp->priormk == 0 && pActTmp->actNum >= RTL8316D_ACLACTINFO_LENGTH)
            {
                return RT_ERR_FILTER_ACTION_NUMBER;
            } else if (pActTmp->priormk == 0){
                pActTmp->actNum++;
                pActTmp->priormk = 1;               
            }
            pActTmp->PriRmkInfo.withdraw = pAction->un.priRmkInfo.withdraw;
            pActTmp->PriRmkInfo.tagSel = pAction->un.priRmkInfo.tagSel;
            pActTmp->PriRmkInfo.inPri = pAction->un.priRmkInfo.inPri;
            pActTmp->PriRmkInfo.outPri = pAction->un.priRmkInfo.outPri;
            pActTmp->PriRmkInfo.dei = pAction->un.priRmkInfo.dei;            
             break;
         
        case ACLACTTYPE_REDIR:
            if (pActTmp->redir == 0 && pActTmp->actNum >= RTL8316D_ACLACTINFO_LENGTH)
            {
                return RT_ERR_FILTER_ACTION_NUMBER;
            } else if (pActTmp->redir == 0){
                pActTmp->actNum++;
                pActTmp->redir = 1;               
            }
            pActTmp->redirInfo.withdraw = pAction->un.redirInfo.withdraw;
            pActTmp->redirInfo.opcode = pAction->un.redirInfo.opcode;
            if (pActTmp->redirInfo.opcode == 0)
            {
                pActTmp->redirInfo.redirect.uniRedirect.dpn = pAction->un.redirInfo.redirect.uniRedirect.dpn;
            } else {
                pActTmp->redirInfo.redirect.multiRedirect.ftIdx = pAction->un.redirInfo.redirect.multiRedirect.ftIdx;
            }
             break;
             
        case ACLACTTYPE_DSCPRMK: 
            if (pActTmp->dscprmk == 0 && pActTmp->actNum >= RTL8316D_ACLACTINFO_LENGTH)
            {
                return RT_ERR_FILTER_ACTION_NUMBER;
            } else if (pActTmp->dscprmk == 0){
                pActTmp->actNum++;
                pActTmp->dscprmk = 1;               
            }
            pActTmp->dscpRmkInfo.withdraw = pAction->un.dscpRmkInfo.withdraw;
            pActTmp->dscpRmkInfo.opcode = pAction->un.dscpRmkInfo.opcode;
            if (pActTmp->dscpRmkInfo.opcode == 0)
            {
                pActTmp->dscpRmkInfo.dscpRmk.dscp.acldscp = pAction->un.dscpRmkInfo.dscpRmk.dscp.acldscp;
            } else if (pActTmp->dscpRmkInfo.opcode == 1)
            {
                pActTmp->dscpRmkInfo.dscpRmk.ipPrec.ipPrece = pAction->un.dscpRmkInfo.dscpRmk.ipPrec.ipPrece;
            } else if (pActTmp->dscpRmkInfo.opcode == 2)
            {
                pActTmp->dscpRmkInfo.dscpRmk.dtr.dtr = pAction->un.dscpRmkInfo.dscpRmk.dtr.dtr;
            } else {
                return RT_ERR_FILTER_ACTION;
            }
         
             break;

        case ACLACTTYPE_PRIORITY: 
            if (pActTmp->prioasn == 0 && pActTmp->actNum >= RTL8316D_ACLACTINFO_LENGTH)
            {
                return RT_ERR_FILTER_ACTION_NUMBER;
            } else if (pActTmp->prioasn == 0){
                pActTmp->actNum++;
                pActTmp->prioasn = 1;               
            }
            pActTmp->prioInfo.withdraw = pAction->un.prioInfo.withdraw;
            pActTmp->prioInfo.priority = pAction->un.prioInfo.priority;
        
             break;

        default:
            return RT_ERR_FILTER_ACTION;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_filter_aclAction_get
 * Description:
 *      Get specified type of ACL action
 * Input:
 *      pAclEntry - ACL rule to be written
 *      actType - action type
 * Output:
 *      pAction - ACL action returned
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 *      RT_ERR_FILTER_ACTION - Invalid action.
 * Note:
 *       The API can get ACL action.      
 */
rtk_api_ret_t rtk_filter_aclAction_get(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType, rtk_filter_aclAction_t* pAction)
{
    rtl8316d_aclAct_entry_t* pActTmp;
    
    if (pAclEntry == NULL || pAction == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (actType >= ACLACTTYPE_END)
    {
        return RT_ERR_FILTER_ACTION;
    }

    pActTmp = (rtl8316d_aclAct_entry_t*)pAclEntry->pAction;
    
    switch(actType)
    {
        case ACLACTTYPE_FNO:                
            pAction->un.fno = pActTmp->fno;
             break;
        
        case ACLACTTYPE_DROP:
            if (pActTmp->drop != 1 && pActTmp->drop != 2)
                return RT_ERR_FILTER_ACTION_NOTFOUND;
             pAction->un.dropInfo = pActTmp->drop;             
             break;
             
        case ACLACTTYPE_CP2CPU: 
            pAction->un.cp2cpu = pActTmp->copytocpu;             
             break;
         
        case ACLACTTYPE_MIRROR:
            pAction->un.mirror = pActTmp->mirror;             
             break;
             
        case ACLACTTYPE_OTAG:
            if (pActTmp->otag == 0 )
            {
                return RT_ERR_FILTER_ACTION_NOTFOUND;
            } 
            
            pAction->un.outTagOpInfo.withdraw = pActTmp->outTagOpInfo.withdraw;
            pAction->un.outTagOpInfo.outVidCtl = pActTmp->outTagOpInfo.outVidCtl;
            pAction->un.outTagOpInfo.outVidInfo= pActTmp->outTagOpInfo.outVidInfo;
            pAction->un.outTagOpInfo.outTagOp = pActTmp->outTagOpInfo.outTagOp;
             break;
         
        case ACLACTTYPE_ITAG:
            if (pActTmp->itag == 0)
            {
                return RT_ERR_FILTER_ACTION_NOTFOUND;
            }
            pAction->un.inTagOpInfo.withdraw = pActTmp->inTagOpInfo.withdraw;
            pAction->un.inTagOpInfo.inVidCtl = pActTmp->inTagOpInfo.inVidCtl;
            pAction->un.inTagOpInfo.inVidInfo = pActTmp->inTagOpInfo.inVidInfo ;
            pAction->un.inTagOpInfo.inTagOp = pActTmp->inTagOpInfo.inTagOp;
             
             break;
             
        case ACLACTTYPE_PRIRMK: 
            if (pActTmp->priormk == 0)
            {
                return RT_ERR_FILTER_ACTION_NOTFOUND;
            }
            
            pAction->un.priRmkInfo.withdraw = pActTmp->PriRmkInfo.withdraw;
            pAction->un.priRmkInfo.tagSel = pActTmp->PriRmkInfo.tagSel;
            pAction->un.priRmkInfo.inPri  = pActTmp->PriRmkInfo.inPri;
            pAction->un.priRmkInfo.outPri = pActTmp->PriRmkInfo.outPri;
            pAction->un.priRmkInfo.dei = pActTmp->PriRmkInfo.dei;
             break;
         
        case ACLACTTYPE_REDIR:
            if (pActTmp->redir == 0)
            {
                return RT_ERR_FILTER_ACTION_NOTFOUND;
            } 
            pAction->un.redirInfo.withdraw = pActTmp->redirInfo.withdraw;
            pAction->un.redirInfo.opcode = pActTmp->redirInfo.opcode;
            if (pActTmp->redirInfo.opcode == 0)
            {
                pAction->un.redirInfo.redirect.uniRedirect.dpn = pActTmp->redirInfo.redirect.uniRedirect.dpn;
            } else {
                pAction->un.redirInfo.redirect.multiRedirect.ftIdx = pActTmp->redirInfo.redirect.multiRedirect.ftIdx;
            }
             break;
             
        case ACLACTTYPE_DSCPRMK: 
            if (pActTmp->dscprmk == 0)
            {
                return RT_ERR_FILTER_ACTION_NOTFOUND;
            }
            pAction->un.dscpRmkInfo.withdraw = pActTmp->dscpRmkInfo.withdraw;
            pAction->un.dscpRmkInfo.opcode = pActTmp->dscpRmkInfo.opcode;
            
            if (pActTmp->dscpRmkInfo.opcode == 0)
            {
                pAction->un.dscpRmkInfo.dscpRmk.dscp.acldscp = pActTmp->dscpRmkInfo.dscpRmk.dscp.acldscp;
            } else if (pActTmp->dscpRmkInfo.opcode == 1)
            {
                pAction->un.dscpRmkInfo.dscpRmk.ipPrec.ipPrece = pActTmp->dscpRmkInfo.dscpRmk.ipPrec.ipPrece; 
            } else
            { 
                pAction->un.dscpRmkInfo.dscpRmk.dtr.dtr = pActTmp->dscpRmkInfo.dscpRmk.dtr.dtr;
            }
         
             break;

        case ACLACTTYPE_PRIORITY: 
            if (pActTmp->prioasn == 0)
            {
                return RT_ERR_FILTER_ACTION_NOTFOUND;
            } 
            pAction->un.prioInfo.withdraw = pActTmp->prioInfo.withdraw;
            pAction->un.prioInfo.priority = pActTmp->prioInfo.priority;

             break;

        default:
            return RT_ERR_FILTER_ACTION;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_aclAction_del
 * Description:
 *      Delete specified type of ACL action
 * Input:
 *      pAclEntry - ACL rule to be written
 *      actType - action type specified
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - 
 * Note:
 *       The API can delete ACL action.      
 */
rtk_api_ret_t rtk_filter_aclAction_del(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType)
{
    rtl8316d_aclAct_entry_t* pActTmp;
    
    if (pAclEntry == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (actType >= ACLACTTYPE_END)
    {
        return RT_ERR_FILTER_ACTION;
    }

    pActTmp = (rtl8316d_aclAct_entry_t*)pAclEntry->pAction;
    
    switch(actType)
    {
        case ACLACTTYPE_FNO: 
             pActTmp->fno = 0;
             break;
        
        case ACLACTTYPE_DROP:
            pActTmp->drop = 0;
             break;
             
        case ACLACTTYPE_CP2CPU: 
            pActTmp->copytocpu = 0;
             break;
         
        case ACLACTTYPE_MIRROR:
            pActTmp->mirror = 0;
             break;
             
        case ACLACTTYPE_OTAG:
            if (pActTmp->otag == 1){
                pActTmp->actNum--;
                pActTmp->otag = 0; 
                break;
            }
         
        case ACLACTTYPE_ITAG:
            if (pActTmp->itag == 1){
                pActTmp->actNum--;
                pActTmp->itag = 0;               
            }

             break;
             
        case ACLACTTYPE_PRIRMK: 
            if (pActTmp->priormk == 1){
                pActTmp->actNum--;
                pActTmp->priormk = 0;               
            }

             break;
         
        case ACLACTTYPE_REDIR:
            if (pActTmp->redir == 1){
                pActTmp->actNum--;
                pActTmp->redir = 0;               
            }

             break;
             
        case ACLACTTYPE_DSCPRMK: 
             if (pActTmp->dscprmk == 1){
                pActTmp->actNum--;
                pActTmp->dscprmk = 0;               
            }
         
             break;

        case ACLACTTYPE_PRIORITY: 
            if (pActTmp->prioasn == 1){
                pActTmp->actNum--;
                pActTmp->prioasn = 0;               
            }

             break;

        default:
            return RT_ERR_FILTER_ACTION;
    }

    return RT_ERR_OK;
}



#if 0
int32 rtl8316d_PieData_setAsicEntry(uint32 index, rtl8316d_tblAsic_templParam_t * data_p, rtl8316d_tblAsic_templParam_t * mask_p)
{
    rtl8316d_tblAsic_template_t * entry;
    uint32 retvalue;
    uint32 asicContent[10];

    if (data_p == NULL || mask_p == NULL)
    {
        return -1;
    }
    if (index < 0 || index >= RTL8316D_PIE_ENTRYNUM)
    {
        rtlglue_printf("input index error %d\n", index);
        return -1;
    }

    memset(asicContent, 0, sizeof(asicContent));

    entry = (rtl8316d_tblAsic_template_t *)asicContent;

    entry->field0 = data_p->field0;
    entry->field1 = data_p->field1;
    entry->field2 = data_p->field2;
    entry->field3 = data_p->field3;
    entry->field4 = data_p->field4;
    entry->field5 = data_p->field5;
    entry->field6 = data_p->field6;
    entry->field7 = data_p->field7;
    entry->field8 = data_p->field8;
    entry->valid = data_p->valid;

    entry = (rtl8316d_tblAsic_template_t *)(asicContent + 5);

    entry->field0 = mask_p->field0;
    entry->field1 = mask_p->field1;
    entry->field2 = mask_p->field2;
    entry->field3 = mask_p->field3;
    entry->field4 = mask_p->field4;
    entry->field5 = mask_p->field5;
    entry->field6 = mask_p->field6;
    entry->field7 = mask_p->field7;
    entry->field8 = mask_p->field8;

    retvalue = table_write(RTL8316D_UNIT, PIE_TABLE, index, asicContent);
    
    if (retvalue != 0)
    {
        rtlglue_printf("write pie data table entry %d failed\n", index);
        return -1;
    }

    return SUCCESS;
}

#endif


