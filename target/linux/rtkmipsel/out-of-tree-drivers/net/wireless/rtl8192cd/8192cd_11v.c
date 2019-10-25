

#ifdef __KERNEL__
#include <linux/module.h>
#include <asm/byteorder.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "../sys-support.h"
#endif

#ifdef __KERNEL__
#include "../ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif

#include "./8192cd_headers.h"
#include "./8192cd_debug.h"
#include "./8192cd.h"
#include "./8192cd_cfg.h"
#include "./8192cd_11v.h"


extern void qsort (void  *base, int nel, int width, int (*comp)(const void *, const void *));
extern unsigned int issue_assocreq(struct rtl8192cd_priv *priv);

#define _8192CD_11V_C_



//++++++++ customized functions 
typedef enum _PREFERENCE_ALGO_ {
	ALGORITHM_0	= 0,
	ALGORITHM_1	= 1
} PREFERENCE_ALGO;

inline unsigned char calculation_method0(bool excluded, unsigned char channel_utilization)
{
	if(excluded)
		return 0;
	return (MAX_PREFERRED_VAL - channel_utilization);
}

unsigned char getPreferredVal(struct rtl8192cd_priv *priv, unsigned char channel_utilization, unsigned char rcpi, bool excluded)
{
	unsigned char retval = 0;
	if(!priv->bssTransPara.FomUser) {
		switch(priv->pmib->wnmEntry.algoType)
		{	
			case ALGORITHM_0:
				retval = calculation_method0(0, channel_utilization);
				break;
			default:
				panic_printk("Undefined Algorithm Type! \n");
				break;
		}
		
		return retval;
	}
}

#ifdef CONFIG_IEEE80211V_CLI

static int validate_target_bssid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i = 0;

	for(i=priv->join_index+1; i<priv->site_survey->count_target; i++) {
		if(!memcmp(pstat->wnm.target_bssid, priv->site_survey->bss_target[i].bssid, MACADDRLEN)) {
			return 0;
		}
	}
	return -1;
}

#endif

//--------- customized functions 

#ifdef CONFIG_RTL_PROC_NEW
int rtl8192cd_proc_transition_list_read(struct seq_file *s, void *data)
#else
int rtl8192cd_proc_transition_list_read(char *buf, char **start, off_t offset,
        int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	int pos = 0;
	int i,j;
	struct stat_info *pstat;

	if((OPMODE & WIFI_AP_STATE) == 0) {
	    panic_printk("\nwarning: invalid command!\n");
	    return pos;
	}

	PRINT_ONE(" --Target Transition List  -- ", "%s", 1);
	j = 1;
	for (i = 0 ; i < MAX_TRANS_LIST_NUM; i++) 
	{
	    if((priv->transition_list_bitmask[i>>3] & (1<<(i&7))) == 0) 
		continue;

	    pstat = get_stainfo(priv, priv->transition_list[i].addr);
	    if(pstat) {
		    PRINT_ONE(j, "  [%d]", 0);
	   	    PRINT_ARRAY_ARG("STA:", priv->transition_list[i].addr, "%02x", MACADDRLEN);
		    PRINT_ONE("    BSS Trans Rejection Count:", "%s", 0);
	           PRINT_ONE(pstat->bssTransRejectionCount, "%d", 1);
	           PRINT_ONE("    BSS Trans Trans Expired Time:", "%s", 0);
	           PRINT_ONE(pstat->bssTransExpiredTime, "%d", 1);
	    }
	    j++;
	}
	
    	return pos;
}

#ifdef __ECOS
int rtl8192cd_proc_transition_list_write(char *tmp, void *data)
#else
int rtl8192cd_proc_transition_list_write(struct file *file, const char *buffer,
        unsigned long count, void *data)
#endif
{
#ifdef __ECOS
	return 0;
#else
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned char error_code = 0;
	char * tokptr;
	int command = 0;
	int empty_slot;
	int i;
	char tmp[TRANS_LIST_PROC_LEN];
	char *tmpptr;
	struct target_transition_list list;

	if((OPMODE & WIFI_AP_STATE) == 0) {
	    error_code = 1;
	    goto end;
	}
	if (count < 2 || count >= TRANS_LIST_PROC_LEN) {
	    return -EFAULT;
	}

	if (buffer == NULL || copy_from_user(tmp, buffer, count))
	    return -EFAULT;

	tmp[count] = 0;
	tmpptr = tmp;
	tmpptr = strsep((char **)&tmpptr, "\n");
	tokptr = strsep((char **)&tmpptr, " ");
	if(!memcmp(tokptr, "add", 3))
		command = 1;
	else if (!memcmp(tokptr, "delall", 6)) 
	   	command = 3;
	else if(!memcmp(tokptr, "del", 3))
       	command = 2;

	if(command) 
	{        
	    if(command == 1 || command == 2) {
	        tokptr = strsep((char **)&tmpptr," ");
	        if(tokptr)
	            get_array_val(list.addr, tokptr, 12);
	        else {
	            error_code = 1;
	            goto end;
	        }
	    }
	    
	    if(command == 1)   /*add*/
	    {	  
			for(i = 0, empty_slot = -1; i < MAX_TRANS_LIST_NUM; i++)
		       {
		            if((priv->transition_list_bitmask[i>>3] & (1<<(i&7))) == 0) {
		            	if(empty_slot == -1)
		            		empty_slot = i;
		            }else if(0 == memcmp(list.addr, priv->transition_list[i].addr, MACADDRLEN)) {
		            	break;
		            }	
		        }
				
		       if(i == MAX_TRANS_LIST_NUM && empty_slot != -1) {/*not found, and has empty slot*/
		        	i = empty_slot;
		        }		
			memcpy(&priv->transition_list[i], &list, sizeof(struct target_transition_list));
		     	priv->transition_list_bitmask[i>>3] |= (1<<(i&7));  
	    }
	    else if(command == 3)   /*delete all*/
	    {
	        	memset(priv->transition_list_bitmask, 0x00, sizeof(priv->transition_list_bitmask));
	    }
	   else if(command == 2)  /*delete*/
	   {
			for (i = 0 ; i < MAX_TRANS_LIST_NUM; i++) {
			        if((priv->transition_list_bitmask[i>>3] & (1<<(i&7))) == 0)
			        	continue;
			    
			        if(0 == memcmp(list.addr, priv->transition_list[i].addr, MACADDRLEN)) {
			        	priv->transition_list_bitmask[i>>3] &= ~(1<<(i&7));
			        	break;
			        }
			}
	    }
	}
	else {
		    error_code = 1;
		    goto end;
	}

	end:
	if(error_code == 1)
	    panic_printk("\nwarning: invalid command!\n");
	else if(error_code == 2)
	    panic_printk("\nwarning: neighbor report table full!\n");
	return count;
#endif
}

void send_bss_trans_event(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char i)
{
	if (priv->pmib->wnmEntry.dot11vDiassocDeadline)
		pstat->expire_to = priv->pmib->wnmEntry.dot11vDiassocDeadline;
	else
		pstat->expire_to = MAX_FTREASSOC_DEADLINE;

#ifdef RTK_SMART_ROAMING
	if(issue_BSS_Trans_Req(priv, priv->bssTransPara.addr, NULL) == SUCCESS ) 
#else
	if(issue_BSS_Trans_Req(priv, priv->transition_list[i].addr, NULL) == SUCCESS)
#endif
	{
		pstat->bssTransExpiredTime = 0;
		pstat->bssTransTriggered = TRUE;
		priv->startCounting = TRUE;

		DOT11VDEBUG("Send BSS Trans Req to STA [SUCCESS]:[%02x][%02x][%02x][%02x][%02x][%02x] \n", 
				priv->transition_list[i].addr[0], priv->transition_list[i].addr[1], priv->transition_list[i].addr[2], 
				priv->transition_list[i].addr[3], priv->transition_list[i].addr[4], priv->transition_list[i].addr[5]);
	}
}

void process_BssTransReq(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	int i, j = 0;
    	unsigned long flags;

	SAVE_INT_AND_CLI(flags);
	for (i = 0; i < MAX_TRANS_LIST_NUM; i++) {
		if((priv->transition_list_bitmask[i>>3] & (1<<(i&7))) == 0) {
			j++;
			continue;
		}
		
#ifdef RTK_SMART_ROAMING
		pstat = get_stainfo(priv, priv->bssTransPara.addr);
#else	
		pstat = get_stainfo(priv, priv->transition_list[i].addr);		
#endif
		if(pstat) {
			if(priv->pmib->wnmEntry.Is11kDaemonOn) {			//collect neighbor report by dot11k daemon
				if(pstat->rcvNeighborReport) 
					send_bss_trans_event(priv, pstat, i);
				else
					panic_printk("Target clients may not Ready yet!!\n");
			}else
				send_bss_trans_event(priv, pstat, i);
		}else
			panic_printk("No such station(%d):[%02x][%02x][%02x][%02x][%02x][%02x] \n", i,
							priv->transition_list[i].addr[0], priv->transition_list[i].addr[1], priv->transition_list[i].addr[2], 
							priv->transition_list[i].addr[3], priv->transition_list[i].addr[4], priv->transition_list[i].addr[5]);
		
	}
	RESTORE_INT(flags);

	if(j == MAX_TRANS_LIST_NUM)
		panic_printk("Transition List is empty !!\n");
	
}

void BssTrans_ExpiredTimer(struct rtl8192cd_priv *priv)
{
	int i;
	unsigned long flags;
	struct stat_info *pstat;

	SAVE_INT_AND_CLI(flags);
	for (i = 0; i < MAX_TRANS_LIST_NUM; i++) {
		if((priv->transition_list_bitmask[i>>3] & (1<<(i&7))) == 0) 
			continue;

#ifdef RTK_SMART_ROAMING
		pstat = get_stainfo(priv, priv->bssTransPara.addr);
#else	
		pstat = get_stainfo(priv, priv->transition_list[i].addr);
#endif
		if(pstat) {
			if(pstat->bssTransTriggered) 	// client does not reply bss trans request
				pstat->bssTransExpiredTime++;

			if(pstat->bssTransExpiredTime == EVENT_TIMEOUT)	
				pstat->bssTransStatusCode = _TIMEOUT_STATUS_CODE_;
		}
	}
	RESTORE_INT(flags);
}

void BssTrans_DiassocTimer(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat;
	unsigned long flags;
	
	if(!priv->pmib->wnmEntry.dot11vDiassocDeadline) 
		priv->startCounting  = FALSE;
	
	if(priv->startCounting  == TRUE && priv->pmib->wnmEntry.dot11vDiassocDeadline) {
		priv->pmib->wnmEntry.dot11vDiassocDeadline--;
		DOT11VDEBUG("Counting down= %d\n", priv->pmib->wnmEntry.dot11vDiassocDeadline);
		if(!priv->pmib->wnmEntry.dot11vDiassocDeadline) {
			SAVE_INT_AND_CLI(flags);
			for (i = 0; i < MAX_TRANS_LIST_NUM; i++) {
				if((priv->transition_list_bitmask[i>>3] & (1<<(i&7))) == 0) 
					continue;

#ifdef RTK_SMART_ROAMING
				pstat = get_stainfo(priv, priv->bssTransPara.addr);
				if(!pstat) {
					panic_printk("Cant find associated STA (%02x%02x%02x%02x%02x%02x)\n",
						priv->bssTransPara.addr[0], priv->bssTransPara.addr[1], priv->bssTransPara.addr[2]
						,priv->bssTransPara.addr[3], priv->bssTransPara.addr[4], priv->bssTransPara.addr[5]);
				} else {
					panic_printk("issue diassoc to trigger bss transition!!\n");
					issue_disassoc(priv, priv->bssTransPara.addr, _RSON_DISASSOC_DUE_BSS_TRANSITION);
					del_station(priv, pstat, 0);		
				}
#else
				pstat = get_stainfo(priv, priv->transition_list[i].addr);
				if(!pstat) {
					panic_printk("Cant find associated STA (%02x%02x%02x%02x%02x%02x)\n",
						priv->transition_list[i].addr[0], priv->transition_list[i].addr[1], priv->transition_list[i].addr[2]
						,priv->transition_list[i].addr[3], priv->transition_list[i].addr[4], priv->transition_list[i].addr[5]);
				} else {
					panic_printk("issue diassoc to trigger bss transition!!\n");
					issue_disassoc(priv, priv->transition_list[i].addr, _RSON_DISASSOC_DUE_BSS_TRANSITION);
					del_station(priv, pstat, 0);		
				}
#endif
				priv->startCounting  = FALSE;
			}   
			RESTORE_INT(flags);
		}
	}
}

void reset_staBssTransStatus(struct stat_info *pstat)
{
	pstat->bssTransRejectionCount = 0;
	pstat->bssTransExpiredTime = 0;
	pstat->rcvNeighborReport = 0;
	pstat->bssTransTriggered = 0;
	pstat->bssTransStatusCode = _DEFAULT_STATUS_CODE_;
}

void set_staBssTransCap(struct stat_info *pstat, unsigned char *pframe, int frameLen, unsigned short ie_offset)
{
	int ie_len = 0;
	unsigned char *ext_cap = NULL;
	unsigned char *p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _EXTENDED_CAP_IE_, &ie_len,  frameLen);

	ext_cap = (unsigned char *)kmalloc(ie_len, GFP_ATOMIC);
	
	if(p != NULL) {
		memcpy(ext_cap, p+2, ie_len);
		if(ext_cap[2] & _WNM_BSS_TRANS_SUPPORT_) {
			pstat->bssTransSupport = TRUE;
		}else {	// when there are 2 extended Capabilities IE (IOT issue with Fujitsu)
			p = p + ie_len + 2;
			if(*p == _EXTENDED_CAP_IE_){
				ie_len = *(p+1);
				memcpy(ext_cap, p+2, ie_len);
				if(ext_cap[2] & _WNM_BSS_TRANS_SUPPORT_)
					pstat->bssTransSupport = TRUE;
			}
		}
	}
	pstat->bssTransStatusCode = _DEFAULT_STATUS_CODE_;
	kfree(ext_cap);
	DOT11VDEBUG("pstat->dot11v_support  %d\n", pstat->bssTransSupport);
}

void set_BssTransPara(struct rtl8192cd_priv *priv, unsigned char *tmpbuf)
{
	int i, empty_slot;

	priv->bssTransPara.FomUser = TRUE;
	
	memcpy(priv->bssTransPara.addr, tmpbuf, MACADDRLEN);
#ifndef RTK_SMART_ROAMING
	priv->bssTransPara.chan_until = tmpbuf[MACADDRLEN];

       for(i = 0, empty_slot = -1; i < MAX_NEIGHBOR_REPORT; i++) {
                if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0) {
                    if(empty_slot == -1)
                        empty_slot = i;
                } else if(0 == memcmp(priv->bssTransPara.addr, priv->rm_neighbor_report[i].bssid, MACADDRLEN)) {
                    break;
                }
        }
        if(i == MAX_NEIGHBOR_REPORT && empty_slot != -1)   /*not found, and has empty slot*/
        {
                i = empty_slot;
        }
#endif
   	DOT11VTRACE("(%s)line=%d, i = %d\n", __FUNCTION__, __LINE__, i);
#ifdef RTK_SMART_ROAMING
	process_BssTransReq(priv);
#endif
}

int issue_BSS_Trans_Req(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char dialog_token)
{	
	int ret;
	unsigned char  *pbuf;
    	unsigned int frlen = 0;
	int neighbor_size = 0, i =0;
	
	unsigned char req_mode = priv->pmib->wnmEntry.dot11vReqMode;
	unsigned short diassoc_time = priv->pmib->wnmEntry.dot11vDiassocDeadline;
	struct stat_info *pstat = get_stainfo(priv, da);
	DECLARE_TXINSN(txinsn);

	DOT11VTRACE("Req mode=%x, diassoc_time = %d, %x\n", req_mode, diassoc_time, cpu_to_le16(diassoc_time));
	
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
	txinsn.fixed_rate = 1;

#ifdef CONFIG_IEEE80211W	
	 if(pstat)
		 txinsn.isPMF = pstat->isPMF;
	 else
		 txinsn.isPMF = 0;
#endif
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
	    goto issue_wnm_bss_trans_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
	    goto issue_wnm_bss_trans_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[frlen++]= _WNM_CATEGORY_ID_; 
	pbuf[frlen++] = _BSS_TSMREQ_ACTION_ID_;

	if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
       	pstat->dialog_token++;	
	
	//dialog_token(1): require mode field(1): dissoc timer(2): validity interval(1)
	if(req_mode & (_WNM_PREFERRED_CANDIDATE_LIST_|_WNM_ABRIDGED_)) {
		pbuf[frlen++] = pstat->dialog_token;
		pbuf[frlen++]  = req_mode; 	
		 *(unsigned short *)(pbuf + frlen) = cpu_to_le16(diassoc_time);
		frlen += 2;
		pbuf[frlen++]  = 200;	//validity interval
		pbuf += frlen;
#ifdef DOT11K
		neighbor_size = sizeof(struct dot11k_neighbor_report);
		for(i = 0; i < MAX_NEIGHBOR_REPORT; i++) {
	   		if((priv->rm_neighbor_bitmask[i>>3] & (1<<(i&7))) == 0)
	    			continue;
	    		if(frlen + neighbor_size > MAX_REPORT_FRAME_SIZE)
	    			break;
	    		
			pbuf = construct_neighbor_report_ie(pbuf, &frlen, &priv->rm_neighbor_report[i]);
	    	}
#endif
	}else {
		panic_printk("Type2 : Bss Trans Req with no neighbor report \n"); 	
		pbuf[frlen++] = pstat->dialog_token;			
		pbuf[frlen++]  = 0; 				
		 *(unsigned short *)(pbuf + frlen) = cpu_to_le16(0);	
		frlen += 2;
		pbuf[frlen++]  = 1;					
		pbuf += frlen;
	}
	
	txinsn.fr_len = frlen;
    	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W	
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
   	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
    	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
   	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	
#if defined(WIFI_WMM)
   	ret = check_dz_mgmt(priv, pstat, &txinsn);
    
    if (ret < 0)
        goto issue_wnm_bss_trans_fail;
    else if (ret==1)
        return 0;
    else
#endif
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) 
		return 0;
	
issue_wnm_bss_trans_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
	return -1;
}

#ifdef CONFIG_IEEE80211V_CLI
static void reset_nieghbor_list_pref_val(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i = 0;
	struct dot11k_neighbor_report *report;

	if(pstat->wnm.num_neighbor_report == 0) {
		panic_printk("(%s)line=%d, Table is empty!! No need to update!! \n", __FUNCTION__, __LINE__); 
		return;
	}
	
	DOT11VTRACE(" Candidate List valid timeout !!\n");
	for (i = 0 ; i < pstat->wnm.num_neighbor_report; i++) {
		report = &pstat->wnm.neighbor_report[i];
		report->subelemnt.preference = priv->pmib->wnmEntry.defaultPrefVal;
	}
}

void BssTrans_ValidatePrefListTimer(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	pstat = get_stainfo(priv, priv->pmib->dot11StationConfigEntry.dot11Bssid);
	if(pstat) {
		if(pstat->wnm.candidate_valid_time > 0) {
			DOT11VTRACE("candidate_valid_time = %d\n", pstat->wnm.candidate_valid_time);
			pstat->wnm.candidate_valid_time--;
			
			if(pstat->wnm.candidate_valid_time == 0) 
				reset_nieghbor_list_pref_val(priv, pstat);
		}	
    	}
}

void BssTrans_TerminationTimer(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	pstat = get_stainfo(priv, priv->pmib->dot11StationConfigEntry.dot11Bssid);
	if(pstat) {
		if(pstat->wnm.dissoc_timer > 0) {
			DOT11VTRACE("bss_termination_duration = %d\n", pstat->wnm.dissoc_timer);
			pstat->wnm.dissoc_timer--;
			
			if(pstat->wnm.candidate_valid_time == 0) {
				DOT11VDEBUG("bss_termination_duration is time up, diassoc to current AP \n");
				//Assume the termination from the AP is collided, so client send diassoc to connecting AP
				issue_disassoc(priv, pstat->hwaddr, _RSON_DISASSOC_DUE_BSS_TRANSITION);
				del_station(priv, pstat, 0);
			}	
		}	
    	}
}

// optional: can use function call this API. At present, it's useless
int  issue_BSS_Trans_Query(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char reason)
{
	int ret;
    	unsigned char *pbuf;
    	unsigned int frlen = 0;	

	DECLARE_TXINSN(txinsn);
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
	txinsn.fixed_rate = 1;

#ifdef CONFIG_IEEE80211W	
	 if(pstat)
		 txinsn.isPMF = pstat->isPMF;
	 else
		 txinsn.isPMF = 0;
#endif
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
	    goto issue_bss_trans_query_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
	    goto issue_bss_trans_query_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
       	pstat->dialog_token++;	
	
	pbuf[frlen++] = _WNM_CATEGORY_ID_;
    	pbuf[frlen++] = _WNM_TSMQUERY_ACTION_ID_;
	pbuf[frlen++] = pstat->dialog_token;
	pbuf[frlen++] = reason;
		
	txinsn.fr_len += frlen;
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
    	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
    	ret = check_dz_mgmt(priv, pstat, &txinsn);
    	if (ret < 0)
       	 goto issue_bss_trans_query_fail;
    	else if (ret==1)
       	 return 0;
    	else
#endif
    	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        	return 0;
		
issue_bss_trans_query_fail:
	if (txinsn.phdr)
       	 release_wlanhdr_to_poll(priv, txinsn.phdr);
    	if (txinsn.pframe)
        	release_mgtbuf_to_poll(priv, txinsn.pframe);

	return -1;
}

int issue_BSS_Trans_Rsp(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char dialog_token, enum bss_trans_mgmt_status_code status)
{
	int ret;
    	unsigned char *pbuf;
    	unsigned int frlen = 0;
    	
	DECLARE_TXINSN(txinsn);
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
	txinsn.fixed_rate = 1;

#ifdef CONFIG_IEEE80211W	
	 if(pstat)
		 txinsn.isPMF = pstat->isPMF;
	 else
		 txinsn.isPMF = 0;
#endif
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
	    goto issue_bss_trans_rsp_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
	    goto issue_bss_trans_rsp_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[frlen++] = _WNM_CATEGORY_ID_;
    	pbuf[frlen++] = _BSS_TSMRSP_ACTION_ID_;
    	pbuf[frlen++] = dialog_token;
	pbuf[frlen++] = status;
	pbuf[frlen++] = 0;	// BSS Termination Delay

	if(pstat->wnm.target_bssid) {
		memcpy(pbuf+frlen, pstat->wnm.target_bssid, MACADDRLEN);
	}else if(status == WNM_BSS_TM_ACCEPT) {
		/*
		 * P802.11-REVmc clarifies that the Target BSSID field is always
		 * present when status code is zero, so use a fake value here if
		 * no BSSID is yet known.
		 */
		memset(pbuf+frlen, 0, MACADDRLEN);
	}
	
	DOT11VDEBUG("Target bssid:[%02x]:[%02x]:[%02x]:[%02x]:[%02x]:[%02x] \n",  
				pstat->wnm.target_bssid[0], pstat->wnm.target_bssid[1], pstat->wnm.target_bssid[2], 
				pstat->wnm.target_bssid[3], pstat->wnm.target_bssid[4], pstat->wnm.target_bssid[5]);

	frlen += MACADDRLEN;
	txinsn.fr_len += frlen;
	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
    	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
    	ret = check_dz_mgmt(priv, pstat, &txinsn);
    	if (ret < 0)
       	 goto issue_bss_trans_rsp_fail;
    	else if (ret==1)
       	 return 0;
    	else
#endif
    	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        	return 0;
		
issue_bss_trans_rsp_fail:
	if (txinsn.phdr)
       	 release_wlanhdr_to_poll(priv, txinsn.phdr);
    	if (txinsn.pframe)
        	release_mgtbuf_to_poll(priv, txinsn.pframe);

	return -1;
}

static int cand_pref_compar(const void *a, const void *b)
{
	const struct dot11k_neighbor_report *aa = a;
	const struct dot11k_neighbor_report *bb = b;

	if (!aa->subelemnt.preference && !bb->subelemnt.preference) {
		return 0;
	}
	if (!aa->subelemnt.preference) {
		return 1;
	}
	if (!bb->subelemnt.preference) {
		return -1;
	}
	
	if (bb->subelemnt.preference > aa->subelemnt.preference)
		return 1;
	if (bb->subelemnt.preference < aa->subelemnt.preference)
		return -1;
	
	return 0;
}

static void update_neighbor_report(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i = 0, num_empty_report = 0;
	unsigned char null_mac[] = {0,0,0,0,0,0};
	
	if(pstat->wnm.num_neighbor_report == 0) {
		panic_printk("(%s)line=%d, Table is empty!! No need to update!! \n", __FUNCTION__, __LINE__); 
		return;
	}

	for(i = 0; i < pstat->wnm.num_neighbor_report; i++) {
		if(!memcmp(pstat->wnm.neighbor_report[i].bssid, null_mac, MACADDRLEN)) {
			num_empty_report++;		
		}
	}
	
	pstat->wnm.num_neighbor_report -= num_empty_report;
	DOT11VTRACE("wnm.num_neighbor_report = %d,   num_empty_report = %d\n", pstat->wnm.num_neighbor_report, num_empty_report);
}

static void start_bss_transition(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if(!memcmp(pstat->wnm.target_bssid, pstat->hwaddr, MACADDRLEN)) {
		DOT11VDEBUG("No need to roam!(The Same Bssid) \n"); 
	} else {
		if(validate_target_bssid(priv, pstat) == 0) {
			DOT11VDEBUG("start bss transition!\n"); 

			// diassoc the present connecting AP
			issue_disassoc(priv, pstat->hwaddr, _RSON_DISASSOC_DUE_BSS_TRANSITION);
			del_station(priv, pstat, 0);

			// set the roaming target
			memcpy(priv->pmib->dot11StationConfigEntry.dot11DesiredBssid ,pstat->wnm.target_bssid, MACADDRLEN);
		}else
			DOT11VDEBUG("Validate NG: Client couldn't find target !\n");
	}
}

static void set_target_bssid(struct stat_info *pstat)
{
	if (pstat->wnm.num_neighbor_report == 0)
		return;
	
	memcpy(pstat->wnm.target_bssid, pstat->wnm.neighbor_report[0].bssid, MACADDRLEN);	// preference highest one
}

// for debug usage
static void dump_cand_list(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char i;
	struct dot11k_neighbor_report *report;
	
	for(i = 0; i < pstat->wnm.num_neighbor_report; i++) {
		report = &pstat->wnm.neighbor_report[i];
		
		panic_printk("(%d)=>[%02x][%02x][%02x][%02x][%02x][%02x], bssinfo=%u, op_class=%u chan=%u phy=%u, prf = %d \n", 
		i, report->bssid[0], report->bssid[1], report->bssid[2], report->bssid[3], report->bssid[4], report->bssid[5],
		report->bssinfo.value, report->op_class, report->channel, report->phytype, report->subelemnt.preference);
	}
}

static void sort_candidate_list(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	 DOT11VDEBUG("Candidate List num = %d\n",pstat->wnm.num_neighbor_report);	
	if (pstat->wnm.num_neighbor_report == 0)
		return;

	qsort(pstat->wnm.neighbor_report, pstat->wnm.num_neighbor_report, sizeof(struct dot11k_neighbor_report), cand_pref_compar);
	update_neighbor_report(priv, pstat);
}

static void parse_subelement(struct rtl8192cd_priv *priv, unsigned char id
							,unsigned char *pos, unsigned char elen, struct dot11k_neighbor_report *report)
{
	report->subelemnt.subelement_id = id;
	
	switch(id) {
		case _WNM_BSS_TRANS_CANDIDATE_PREFRENCE_:
			if(elen < 1) {
				panic_printk("Too short BSS transition candidate \n");
				break;
			}
			report->subelemnt.len = elen;
			report->subelemnt.preference = pos[0];
			break;
		default:			
			panic_printk("Not implemented subelement id! \n");
			break;
	}
}

static void parse_neighbor_report(struct rtl8192cd_priv *priv, unsigned char *pos, unsigned char len, 
									struct dot11k_neighbor_report *report)
{
	unsigned char left = len;
	if(left < NEIGHBOR_REPORT_SIZE) {
		panic_printk("Too short neighbor report \n");
		return;
	}

	memcpy(report->bssid, pos, MACADDRLEN);
	pos += MACADDRLEN;	
	report->bssinfo.value = le32_to_cpu(*(unsigned int *)&pos[0]);
	pos += sizeof(int);		
	report->op_class = *pos;
	pos++;				
	report->channel = *pos;
	pos++;				
	report->phytype = *pos;
	pos++;			

	left -= 13; 
	
	DOT11VTRACE("[%02x][%02x][%02x][%02x][%02x][%02x], bssinfo=%u, op_class=%u chan=%u phy=%u \n",
		report->bssid[0], report->bssid[1], report->bssid[2], report->bssid[3], report->bssid[4], report->bssid[5],
		report->bssinfo.value, report->op_class, report->channel, report->phytype);
	while(left >= 2) {
		unsigned char id, elen;
		id = *pos++;
		elen = *pos++;
		
		left -= 2;	
		if(elen > left) {
			DOT11VDEBUG("Truncated neighbor report subelement \n");
			break;
		}
		parse_subelement(priv, id, pos, elen, report);
		left -= elen;
		pos += elen;
	}
	
}

void OnBSSTransReq(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len)
{
	int frlen = 0;
	enum bss_trans_mgmt_status_code status;
	unsigned char valid_int;
	unsigned int beacon_int = 100;	
	unsigned char *pos = (pframe+2);

	if(frame_len <5) {
		panic_printk(" Ignore too short BSS Transition Management Request!\n"); 
		return;
	}

	pstat->wnm.dialog_token = pos[0];
	pstat->wnm.req_mode = pos[1];
	pstat->wnm.dissoc_timer = le16_to_cpu(*(unsigned short *) &pos[2]); 
	pstat->wnm.reply = TRUE;
	valid_int = pos[4];
	
	DOT11VDEBUG("BSS Trans Req: dialog_token=%u, req_mode=0x%x, dissoc_timer=%u, valid_int=%u, frame_len = %d \n",
		   			pstat->wnm.dialog_token, pstat->wnm.req_mode, pstat->wnm.dissoc_timer, valid_int, frame_len);

	pos += 5;
	frlen += 7;
	if(pstat->wnm.req_mode & _WNM_BSS_TERMINATION_INCLUDED_) {
		DOT11VTRACE("BSS termination included \n");
		if(frlen + 12 > frame_len) {	//Bss Trans duration 0 or 12
			panic_printk("Too short BSS TM Request \n");
			return;
		}
		memcpy(pstat->wnm.bss_termination_duration, pos, BSS_TERMINATION_DURATION_LEN);
		pos  += BSS_TERMINATION_DURATION_LEN;
		frlen += BSS_TERMINATION_DURATION_LEN;
	}

	if(pstat->wnm.req_mode & _WNM_ESS_DIASSOC_IMMINENT_) {
		unsigned char url[256];
		if (frlen + 1 > frame_len || frlen + 1 + pos[1] > frame_len) {
			panic_printk("Invalid BSS Transition Management Request (URL)");
			return;
		}
		memcpy(url, pos+1, pos[0]); 
		url[pos[0]] ='\0';
		pos += 1 + pos[0];
		frlen += 1 + pos[0];
		DOT11VTRACE("ESS_DISASSOC_IMMINENT(disassoc_timer: %u, url: %s)", pstat->wnm.dissoc_timer * beacon_int * 128 / 125, url);
	}

	if(pstat->wnm.req_mode & _WNM_DIASSOC_IMMINENT_) {
		DOT11VTRACE("Disassociation Imminent: dissoc_timer:%u\n", pstat->wnm.dissoc_timer);
		if(pstat->wnm.dissoc_timer) {
			start_clnt_lookup(priv, 1); //start rescan(needed to check)
		}
	}

	if(pstat->wnm.req_mode & _WNM_PREFERRED_CANDIDATE_LIST_) {
		DOT11VTRACE("PREFERRED_CANDIDATE_LIST: \n");
		unsigned int valid_ms;
		
		int neighbor_size = sizeof(struct dot11k_neighbor_report);
		memset(pstat->wnm.neighbor_report, 0, neighbor_size * MAX_NEIGHBOR_REPORT);	

		while((frlen + 2 <= frame_len) &&
		   	pstat->wnm.num_neighbor_report < MAX_NEIGHBOR_REPORT)
		 {	
	   		unsigned char tag = *pos++;
			unsigned char len = *pos++;
			frlen += 2;

			if(frlen + len > frame_len) {
				panic_printk("Truncated request size");
				return;		
			}

			if(tag == _NEIGHBOR_REPORT_IE_) {
				struct dot11k_neighbor_report *report;
				report = &pstat->wnm.neighbor_report[pstat->wnm.num_neighbor_report];
				parse_neighbor_report(priv, pos, len, report);
			}
				
			pos += len;
			frlen += len;
			pstat->wnm.num_neighbor_report++;
		 }
		
		 sort_candidate_list(priv, pstat);
#ifdef DOT11V_DEBUG
		 dump_cand_list(priv, pstat);
#endif
		 set_target_bssid(pstat);		 
		 valid_ms = valid_int * beacon_int * 128 / 125;
		 pstat->wnm.candidate_valid_time = (valid_ms)/1000; 
		 DOT11VTRACE("Candidate list valid for (%d) ms/(%d) sec\n", valid_ms, pstat->wnm.candidate_valid_time);				
	}
	
	if(pstat->wnm.reply) {		
		if(pstat->wnm.req_mode & _WNM_PREFERRED_CANDIDATE_LIST_)
			status = WNM_BSS_TM_ACCEPT;
		else {
			DOT11VDEBUG("BSS Transition Request did not include candidates \n");
			status = WNM_BSS_TM_REJECT_UNSPECIFIED;	
		}

		if(issue_BSS_Trans_Rsp(priv, pstat, pstat->wnm.dialog_token, status) == 0)
			start_bss_transition(priv, pstat);
	}
	return;
}

#endif

static void process_status_code( struct stat_info *pstat, unsigned char status_code)
{
	pstat->bssTransStatusCode = status_code;
	
	if(status_code)		
		pstat->bssTransRejectionCount++;

	switch(status_code) {
		case WNM_BSS_TM_ACCEPT:
			panic_printk("Accept: WNM_BSS_TM_ACCEPT![%02x][%02x][%02x][%02x][%02x][%02x] \n", 
						pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]); 
			
			pstat->bssTransRejectionCount = 0;
			break;
		case WNM_BSS_TM_REJECT_UNSPECIFIED:
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_UNSPECIFIED!\n"); 
			break;
		case WNM_BSS_TM_REJECT_INSUFFICIENT_BEACON: 
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_INSUFFICIENT_BEACON!\n"); 
			break;
		case WNM_BSS_TM_REJECT_INSUFFICIENT_CAPABITY:
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_INSUFFICIENT_CAPABITY!\n"); 
			break;
		case WNM_BSS_TM_REJECT_UNDESIRED :
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_UNDESIRED !\n"); 
			break;
		case WNM_BSS_TM_REJECT_DELAY_REQUEST : 
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_DELAY_REQUEST !\n"); 
			break;
		case WNM_BSS_TM_REJECT_STA_CANDIDATE_LIST_PROVIDED :
			panic_printk("Reject: WNM_BSS_TM_REJECT_STA_CANDIDATE_LIST_PROVIDED !\n"); 
			break;
		case WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES: 
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES  !\n"); 
			break;
		case WNM_BSS_TM_REJECT_LEAVING_ESS:
			DOT11VDEBUG("Reject: WNM_BSS_TM_REJECT_LEAVING_ESS  !\n"); 
			break;
		default:
			DOT11VDEBUG("unknown type !\n"); 
			break;
	}
}

void OnBSSTransRsp(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char*pframe, int frame_len)
{
	int frlen = 0;

	if(frame_len < 3) {		
		panic_printk("Ignore too short BSS Trans Management RSP!\n"); 
		return;
	}
	
	unsigned char dialog_token = pframe[2];
	unsigned char status_code = pframe[3];
	unsigned char bss_termination_delay = pframe[4];	//mins
	frlen = 5;

	if((frame_len - frlen) > MAX_LIST_LEN)	
		return;
	
	DOT11VTRACE("dialog_token = %d, bss_termination_delay = %d\n", dialog_token, bss_termination_delay);
	process_status_code(pstat, status_code);
	pstat->bssTransExpiredTime = 0;	
	pstat->bssTransTriggered = 0;
	
	if((frame_len - frlen) > 0) {	
		if(status_code == WNM_BSS_TM_ACCEPT) {
			debug_out("Target BSSID: ", &pframe[frlen], MACADDRLEN);
			frlen += MACADDRLEN;
			debug_out("Bss Trans Candidate List: ", &pframe[frlen], frame_len - frlen);
		}else if(status_code == WNM_BSS_TM_REJECT_DELAY_REQUEST){
			priv->pmib->wnmEntry.dot11vDiassocDeadline = bss_termination_delay*60;
		}else
			debug_out("Bss Trans Candidate List: ", &pframe[frlen], frame_len - frlen);
	}else {
		DEBUG_ERR("WNM: no info in bss trans response!\n");
	}
	
	return;
}

void OnBSSTransQuery(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char*pframe, int frame_len)
{
	int list_len;
	
	if(frame_len < 2) {
		DEBUG_ERR("Ignore too short BSS Transition Management Query!\n"); 		
		return;
	}
	
	unsigned char dialog_token = pframe[2];
	unsigned char reason  = pframe[3]; 

	DOT11VTRACE("dialog_token = %d, reason = %d\n", dialog_token, reason); 
	
 	if(reason == _WNM_PREFERED_BSS_TRANS_LIST_INCLUDED_) {
		list_len =   frame_len - 4;
		
		if(list_len > MAX_LIST_LEN)
			return;
		debug_out("Bss List Len: ", &pframe[list_len], frame_len - list_len);
	} else
		panic_printk("WNM_PREFERED BSS TRANS LIST NOT INCLUDED!\n"); 

	issue_BSS_Trans_Req(priv, pstat->hwaddr, dialog_token);
}

void WNM_ActionHandler(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe, int frame_len)
{
	unsigned char action_field = pframe[1];
	
	switch (action_field) {
		case _WNM_TSMQUERY_ACTION_ID_: 
			OnBSSTransQuery(priv, pstat, pframe, frame_len);
			break;
		case _BSS_TSMRSP_ACTION_ID_:
			OnBSSTransRsp(priv, pstat, pframe, frame_len);
			break;
	#ifdef CONFIG_IEEE80211V_CLI
		case _BSS_TSMREQ_ACTION_ID_:
			OnBSSTransReq(priv, pstat, pframe, frame_len);
			break;
	#endif
		default:
			DEBUG_INFO("Other WNM action: %d:\n", action_field);
			break;
	}
}

