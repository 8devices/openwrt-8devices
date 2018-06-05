#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "assert.h"
#include "asicRegs.h"
//#include "rtl_utils.h"
//#include "rtl8651_debug.h"
#include "rtl865xC_hs.h"


static int32 convertHsbToSoftware( hsb_t* rawHsb, hsb_param_t* hsb )
{
	/* bit-to-bit mapping */
	hsb->spa = rawHsb->spa;
	hsb->trigpkt = rawHsb->trigpkt;

#if	defined(CONFIG_RTL_8197F)
	hsb->ipv4_opt = rawHsb->ipv4_opt;			/* W0[4]		1 */
	hsb->len = rawHsb->len;						/* W0[18:5]		14 */
	hsb->ppp_ctrl = rawHsb->ppp_ctrl;			/* W0[19]		1 */
#else
 	hsb->len = rawHsb->len;						/* W0[19:5]		15 */
#endif

	hsb->vid = rawHsb->vid;
	hsb->tagif = rawHsb->tagif;
	hsb->pppoeif = rawHsb->pppoeif;
	hsb->sip = rawHsb->sip29_0 | (rawHsb->sip31_30<<30);
	hsb->sprt = rawHsb->sprt;
	hsb->dip = rawHsb->dip13_0 | (rawHsb->dip31_14<<14);
	hsb->dprt = rawHsb->dprt13_0 | (rawHsb->dprt15_14<<14);
	hsb->ipptl = rawHsb->ipptl;
	hsb->ipfg = rawHsb->ipfg;
	hsb->iptos = rawHsb->iptos;
	hsb->tcpfg = rawHsb->tcpfg;
	hsb->type = rawHsb->type;
	hsb->patmatch = rawHsb->patmatch;
	hsb->ethtype = rawHsb->ethtype;
#if 1 /* Since the endian is reversed, we must translate it. */
	hsb->da[5] = rawHsb->da14_0;
	hsb->da[4] = (rawHsb->da14_0>>8) | (rawHsb->da46_15<<7);
	hsb->da[3] = rawHsb->da46_15>>1;
	hsb->da[2] = rawHsb->da46_15>>9;
	hsb->da[1] = rawHsb->da46_15>>17;
	hsb->da[0] = (rawHsb->da46_15>>25) | (rawHsb->da47_47<<7);
	hsb->sa[5] = rawHsb->sa30_0;
	hsb->sa[4] = rawHsb->sa30_0>>8;
	hsb->sa[3] = rawHsb->sa30_0>>16;
	hsb->sa[2] = (rawHsb->sa30_0>>24) | (rawHsb->sa47_31<<7);
	hsb->sa[1] = rawHsb->sa47_31>>1;
	hsb->sa[0] = rawHsb->sa47_31>>9;
#else
	hsb->da[0] = rawHsb->da14_0;
	hsb->da[1] = (rawHsb->da14_0>>8) | (rawHsb->da46_15<<7);
	hsb->da[2] = rawHsb->da46_15>>1;
	hsb->da[3] = rawHsb->da46_15>>9;
	hsb->da[4] = rawHsb->da46_15>>17;
	hsb->da[5] = (rawHsb->da46_15>>25) | (rawHsb->da47_47<<7);
	hsb->sa[0] = rawHsb->sa30_0;
	hsb->sa[1] = rawHsb->sa30_0>>8;
	hsb->sa[2] = rawHsb->sa30_0>>16;
	hsb->sa[3] = (rawHsb->sa30_0>>24) | (rawHsb->sa47_31<<7);
	hsb->sa[4] = rawHsb->sa47_31>>1;
	hsb->sa[5] = rawHsb->sa47_31>>9;
#endif
	hsb->hiprior = rawHsb->hiprior;
	hsb->snap = rawHsb->snap;
	hsb->udpnocs = rawHsb->udpnocs;
	hsb->ttlst = rawHsb->ttlst;
	hsb->dirtx = rawHsb->dirtx;
	hsb->l3csok = rawHsb->l3csok;
	hsb->l4csok = rawHsb->l4csok;
	hsb->ipfo0_n = rawHsb->ipfo0_n;
	hsb->llcothr = rawHsb->llcothr;
	hsb->urlmch = rawHsb->urlmch;
	hsb->extspa = rawHsb->extspa;
	hsb->extl2 = rawHsb->extl2;
	hsb->linkid = rawHsb->linkid;
	hsb->pppoeid = rawHsb->pppoeid;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	hsb->cputag_if = rawHsb->cputag_if;
    hsb->ipv6_ext  = rawHsb->ipv6_ext;
    hsb->ipv6fo    = rawHsb->ipv6fo;
    hsb->ipv6flag  = rawHsb->ipv6flag;
    hsb->qpri      = (rawHsb->qpri2_1<<1)| rawHsb->qpri0; 
	hsb->ptp_pkt   = rawHsb->ptp_pkt;
	hsb->ptp_ver   = rawHsb->ptp_ver;
	hsb->ptp_typ   = rawHsb->ptp_typ;
	hsb->ipver_1st = rawHsb->ipver_1st;
    
    hsb->sipv6_31_0 =   ((rawHsb->sipv6_50_19&0x1FFF)<<19)  | rawHsb->sipv6_18_0;
    hsb->sipv6_63_32 =  ((rawHsb->sipv6_82_51&0x1FFF)<<19)  | (rawHsb->sipv6_50_19>>13);
    hsb->sipv6_95_64 =  ((rawHsb->sipv6_114_83&0x1FFF)<<19)  | (rawHsb->sipv6_82_51>>13);
    hsb->sipv6_127_96 = ((rawHsb->sipv6_127_115&0x1FFF)<<19)  | (rawHsb->sipv6_114_83>>13);

    hsb->dipv6_31_0 =   ((rawHsb->dipv6_50_19&0x1FFF)<<19)  | rawHsb->dipv6_18_0;
    hsb->dipv6_63_32 =  ((rawHsb->dipv6_82_51&0x1FFF)<<19)  | (rawHsb->dipv6_50_19>>13);
    hsb->dipv6_95_64 =  ((rawHsb->dipv6_114_83&0x1FFF)<<19)  | (rawHsb->dipv6_82_51>>13);
    hsb->dipv6_127_96 = ((rawHsb->dipv6_127_115&0x1FFF)<<19)  | (rawHsb->dipv6_114_83>>13);

	hsb->hop_limit    = rawHsb->hop_limit;
	hsb->tra_cla      = rawHsb->tra_cla;
    hsb->flow_lab     = (rawHsb->flow_lab_19_3<<3) | rawHsb->flow_lab_2_0;
	hsb->nxt_hdr      = rawHsb->nxt_hdr;
	hsb->ipv4         = rawHsb->ipv4;
    hsb->ipv6         = rawHsb->ipv6; 
    hsb->ip_len       = (rawHsb->ip_len_15_5<<5)| rawHsb->ip_len_4_0; 

    #if defined(CONFIG_RTL_8197F)
    hsb->l3_cso = rawHsb->l3_cso;				/* W20[11]		1 */
    hsb->l4_cso = rawHsb->l4_cso;				/* W20[12]		1 */
    #endif

    hsb->tun_len      = rawHsb->tun_len;

    #if defined(CONFIG_RTL_8197F)
    hsb->l3_csum = rawHsb->l3_csum_4_0;			/* W20[31:27]	5 */
    /* W21 */
    hsb->l3_csum |=								/* W21[10:0]	11 */
        (rawHsb->l3_csum_15_5 << 5);
    hsb->l4_csum = rawHsb->l4_csum;				/* W21[26:11]	16 */
    hsb->inn_hlen = rawHsb->inn_hlen_4_0;		/* W21[31:27]	5 */
    /* W22 */
    hsb->inn_hlen |= rawHsb->inn_hlen_13_5;		/* W22[8:0]		9 */
    hsb->swred_pkt_flag =						/* W22[9]		1 */
        rawHsb->swred_pkt_flag;
    hsb->swred_pkt_type =						/* W22[11:10]	2 */
        rawHsb->swred_pkt_type;
    #endif
#endif  

	return SUCCESS;
}


int32 virtualMacGetHsb( hsb_param_t* hsb )
{
	hsb_t rawHsb;
	int32 ret = SUCCESS;

	{ /* Word-Access */
		uint32 *pSrc, *pDst;
		uint32 i;

		/* We must assert structure size is the times of 4-bytes. */
		if ( (sizeof(rawHsb)%4) != 0 ) RTL_BUG( "sizeof(rawHsb) is not the times of 4-bytes." );

		pSrc = (uint32*)HSB_BASE;
		pDst = (uint32*)&rawHsb;
		for( i = 0; i < sizeof(rawHsb); i+=4 )
		{
			*pDst = READ_MEM32((uint32)pSrc);
			pSrc++;
			pDst++;
		}
	}

	convertHsbToSoftware( &rawHsb, hsb );
	return ret;
}

int32 convertHsaToSoftware( hsa_t* rawHsa, hsa_param_t* hsa )
{
	/* bit-to-bit mapping */
#if 1 /* Since the endian is reversed, we must translate it. */
	hsa->nhmac[5] = rawHsa->nhmac0;
	hsa->nhmac[4] = rawHsa->nhmac1;
	hsa->nhmac[3] = rawHsa->nhmac2;
	hsa->nhmac[2] = rawHsa->nhmac3;
	hsa->nhmac[1] = rawHsa->nhmac4;
	hsa->nhmac[0] = rawHsa->nhmac5;
#else
	hsa->nhmac[0] = rawHsa->nhmac0;
	hsa->nhmac[1] = rawHsa->nhmac1;
	hsa->nhmac[2] = rawHsa->nhmac2;
	hsa->nhmac[3] = rawHsa->nhmac3;
	hsa->nhmac[4] = rawHsa->nhmac4;
	hsa->nhmac[5] = rawHsa->nhmac5;
#endif

	hsa->trip = rawHsa->trip15_0 | (rawHsa->trip31_16<<16);
	hsa->port = rawHsa->port;
	hsa->l3csdt = rawHsa->l3csdt;
	hsa->l4csdt = rawHsa->l4csdt;
	hsa->egif = rawHsa->egif;
	hsa->l2tr = rawHsa->l2tr;
	hsa->l34tr = rawHsa->l34tr;
	hsa->dirtxo = rawHsa->dirtxo;
	hsa->typeo = rawHsa->typeo;

#if defined(CONFIG_RTL_8197F)
	hsa->llcexist = rawHsa->llcexist;			/* W4[7]		1 */
#else
	hsa->snapo = rawHsa->snapo;
#endif
	hsa->rxtag = rawHsa->rxtag;
	hsa->dvid = rawHsa->dvid;
	hsa->pppoeifo = rawHsa->pppoeifo;
	hsa->pppidx = rawHsa->pppidx;
#if defined(CONFIG_RTL_8197F)
	hsa->leno = rawHsa->leno5_0|(rawHsa->leno13_6<<6);
#else
	hsa->leno = rawHsa->leno5_0|(rawHsa->leno14_6<<6);
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    hsa->mirrort = rawHsa->mirrort; 
#endif
	hsa->l3csoko = rawHsa->l3csoko;
	hsa->l4csoko = rawHsa->l4csoko;
	hsa->frag = rawHsa->frag;
	hsa->lastfrag = rawHsa->lastfrag;
	hsa->ipmcastr = rawHsa->ipmcastr;
	hsa->svid = rawHsa->svid;

#if defined(CONFIG_RTL_8197F)
	hsa->_6rd_df = rawHsa->_6rd_df;				/* W5[26]		1 */
#else
	hsa->fragpkt = rawHsa->fragpkt;
#endif
	hsa->ttl_1if = rawHsa->ttl_1if4_0|(rawHsa->ttl_1if5_5<<5)|(rawHsa->ttl_1if8_6<<6);
	hsa->dpc = rawHsa->dpc;
	hsa->spao = rawHsa->spao;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    hsa->ipv4_1st = rawHsa->ipv4_1st;
#endif
	hsa->hwfwrd = rawHsa->hwfwrd;
	hsa->dpext = rawHsa->dpext;
	hsa->spaext = rawHsa->spaext;
	hsa->why2cpu = rawHsa->why2cpu13_0|(rawHsa->why2cpu15_14<<14);
	hsa->spcp = rawHsa->spcp;
	hsa->dvtag = rawHsa->dvtag;
	hsa->difid = rawHsa->difid;
	hsa->linkid = rawHsa->linkid;
	hsa->siptos = rawHsa->siptos;
	hsa->dp = rawHsa->dp6_0;
	hsa->priority = rawHsa->priority;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8881A)
	hsa->cputag   = rawHsa->cputag;
	hsa->ptp_pkt  = rawHsa->ptp_pkt;
	hsa->ptp_v2   = rawHsa->ptp_v2;
	hsa->ptp_type = rawHsa->ptp_type;
	hsa->rmdp     = rawHsa->rmdp;
	hsa->dpri     = rawHsa->dpri;
#elif defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	hsa->cputag   = rawHsa->cputag;
	hsa->ptp_pkt  = rawHsa->ptp_pkt;
	hsa->ptp_v2   = rawHsa->ptp_v2;
	hsa->ptp_type = rawHsa->ptp_type;
	hsa->rmdp     = rawHsa->rmdp;
	hsa->dpri     = rawHsa->dpri;
	hsa->mdf      = rawHsa->mdf;
  
    hsa->sipv6_31_0   = ((rawHsa->sipv6_43_12  &0xFFFFF)<<12) | rawHsa->sipv6_11_0;
    hsa->sipv6_63_32  = ((rawHsa->sipv6_75_44  &0xFFFFF)<<12) |(rawHsa->sipv6_43_12>>20);
    hsa->sipv6_95_64  = ((rawHsa->sipv6_107_76 &0xFFFFF)<<12) |(rawHsa->sipv6_75_44>>20);
    hsa->sipv6_127_96 = ((rawHsa->sipv6_127_108&0xFFFFF)<<12) |(rawHsa->sipv6_107_76>>20);

    hsa->dipv6_31_0   = ((rawHsa->dipv6_43_12  &0xFFFFF)<<12) | rawHsa->dipv6_11_0;
    hsa->dipv6_63_32  = ((rawHsa->dipv6_75_44  &0xFFFFF)<<12) |(rawHsa->dipv6_43_12>>20);
    hsa->dipv6_95_64  = ((rawHsa->dipv6_107_76 &0xFFFFF)<<12) |(rawHsa->dipv6_75_44>>20);
    hsa->dipv6_127_96 = ((rawHsa->dipv6_127_108&0xFFFFF)<<12) |(rawHsa->dipv6_107_76>>20);

    hsa->ip_len    = (rawHsa->ip_len_15_12<<12)| rawHsa->ip_len_11_0;
    hsa->ipv4_id   = rawHsa->ipv4_id; 
    #if defined(CONFIG_RTL_8197F)
    hsa->has_ipv4 = rawHsa->has_ipv4;			/* W19[20]		1 */
    hsa->has_ipv6 = rawHsa->has_ipv6;			/* W19[21]		1 */
    hsa->tun_len = rawHsa->tun_len_9_0;			/* W19[31:22]	10 */

    hsa->tun_len |=	(rawHsa->tun_len_13_10<<10); /* W20[3:0]		4 */		
    #else
    hsa->tun_len   = (rawHsa->tun_len_15_12<<12) | rawHsa->tun_len_11_0 ;
    #endif
    hsa->mltcst_v6 = rawHsa->mltcst_v6;
    hsa->addip_pri = rawHsa->addip_pri;
#endif

#if defined(CONFIG_RTL_8197F)
	hsa->l3_cso = rawHsa->l3_cso;				/* W20[8]		1 */
	hsa->l4_cso = rawHsa->l4_cso;				/* W20[9]		1 */
	hsa->inn_hlen = rawHsa->inn_hlen;			/* W20[23:10]	14 */
	hsa->swred_pkt_flag =						/* W20[24]		1 */
		rawHsa->swred_pkt_flag;
	hsa->swred_pkt_type =						/* W20[26:25]	2 */
		rawHsa->swred_pkt_type;
#endif

	return SUCCESS;
}

int32 virtualMacGetHsa( hsa_param_t* hsa )
{
	hsa_t rawHsa;
	int32 ret = SUCCESS;

	{ /* Word-Access */
		uint32 *pSrc, *pDst;
		uint32 i;

		/* We must assert structure size is the times of 4-bytes. */
		if ( (sizeof(rawHsa)%4) != 0 ) RTL_BUG( "sizeof(rawHsa) is not the times of 4-bytes." );

		pSrc = (uint32*)HSA_BASE;
		pDst = (uint32*)&rawHsa;
		for( i = 0; i < sizeof(rawHsa); i+=4 )
		{
			*pDst = READ_MEM32((uint32)pSrc);
			pSrc++;
			pDst++;
		}
	}

	convertHsaToSoftware( &rawHsa, hsa );
	return ret;
}
