
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of all error numbers
* Abstract :                                                           
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)               
* $Id: rtl_errno.h,v 1.1 2007-12-21 10:29:52 davidhsu Exp $
* $log$
* -------------------------------------------------------
*/

#ifndef _RTL_ERRNO_H
#define _RTL_ERRNO_H

//Driver ERROR NUMBER Definition

/* Error Number for common use */
#define RTL_DUMMY						-1 // NEVER use me!!! '-1' is used by FAILED.
#define RTL_EENTRYALREADYEXIST			-2 //The specifyied entry already exists.
#define RTL_EENTRYNOTFOUND			-3 //The specified entry was not found.
#define RTL_EINVALIDPORT				-4 //Unknown port number
#define RTL_EINVALIDVLANID				-5 //Invalid VLAN ID.
#define RTL_EINVALIDINPUT				-6 //Invalid input parameter.
#define RTL_EINVNETIFNAME				-7 //Invalid Network Interface name
#define RTL_ENEEDLOOPBACKSET			-8 //Need loopback port set
#define RTL_ENOFREEBUFFER				-9 //No Free buffer for new request
#define RTL_EINVALIDIPRANGE				-10//Invalid IP RANGE
#define RTL_EINVALIDIPFILTER			-11//Invalid Ip filter which for filter pkt's sip or dip
#define RTL_ETOOMANYCOLLISION			-12//too many adding collsion

/* Error Number for Port Mirror */
#define RTL_EPORTISAMIRRORPORT		-1000 //The specified port is already configured as a mirror port
#define RTL_EPORTISARXTXMIRRORED			-1001 //The specified port is a Rx/Tx mirrored port
#define RTL_EPORTCANTBEAGGREGATED	-1002 //The Mirror port cannot be aggregated.
#define RTL_EMIRRORPORTNOTFOUND		-1003 //The specified Mirror port not found.

/* DMZ Host error number */
#define RTL_EDMZHOSTNOTFOUND			-1100 //DMZ host not found
#define RTL_EDMZHOSTNEEDNAPTIP		-1101 //DMZ host needs NAPT Ip address present
#define RTL_EDMZHOSTSHOULDBEINTIP		-1102 //DMZ host should be internal ip address
#define RTL_EDMZHOSTCANNOTBEGWIP		-1103 //DMZ host cannot be gateway ip address
#define RTL_EDMZHOSTFULL				-1104 //DMZ host array is full.
#define RTL_EDMZHOSTDENYFILTEREDSIP	-1105 //filtered PKT to DMZ because of the PKT's Sip in filterring range

/* Error Number for ALG APIs */
#define RTL_EINVALALGATTR			-1200 //Invalid ALG attribute. Neither Server nor Client is 
											  //specified to an ALG entry.
#define RTL_EINVALPORTRANGE			-1201 //Invalid port range was specified.
#define RTL_EALGPORTRANGEOVERLAP		-1202 //ALG port range overlap

/* Error Number for Protocol Trap APIs */
#define RTL_EUNKNOWPROTOTYPE			-1300 //Unknown Protocol Trap's Protocol type


/* Error Number for ACL rule APIs */
#define RTL_EACLRULEISNULL			-1400 //The specified ACL rule is NULL
#define RTL_EINVALACTIONTYPE			-1401 //ACL rule with invalid action type
#define RTL_EACLMODEISNOTALLOWED		-1402 //ACL mode is not match.
#define RTL_EACLRULE_MATCHTYPEINCORRECT 	-1403 /* Incorrect match type */
#define RTL_EACLEGRESSRULE_NOTSUPPORTPKTOP	-1404	/* Egress ACL not support packet operation */
#define RTL_EACLINVALIDRULETYPE				-1405	/* Invalid acl rule type */

/* Error Number for NAPT APIs */
#define RTL_ENAPTIPNOTEXTIP			-1500 //The Specified NAPT IP is not an external IP address
#define RTL_ENAPTIPCANNOTBENAT		-1501 //The Specified NAPT IP can not a NAT IP address

/* Error Number for Ethernet port */
#define RTL_EPORTOPMODECANNOTAGGREGATE	-1600 //Can not set a port's operation mode if the port is aggreaged.
#define RTL_EUNKNOWPORTSPEED				-1601 //Unknow port speed. The port speed should be either 10 or 100.
#define RTL_EPORTSPEEDCANNOTAGGREGATE 	-1602 //The specified port's speed can not be changed if the port is aggreaged.
#define RTL_EPORTAUTONEGOCANNOTAGGREGATE	-1603 //Can not change a port's auto-negotiation capability if the port is aggregated.
#define RTL_EINVALIDDATARATE				-1604 //Invalid data rate.
#define RTL_ENOTVLANPORTMEMBER			-1605 //Specified port is not a member port of specified vlan.

/* Error Number for Spanning Tree */
#define RTL_EINVALIDSID					-1700 //Invalid spanning tree ID.
#define RTL_ESIDALREADYEXIST				-1701 //Spannding Tree ID already exists.
#define RTL_EINVALIDPORTSTATE			-1702 //Invalid Port State.
#define RTL_ESIDISREFERENCEEDBYFID		-1703 //Spanning Tree is being referenced by Filter Database.

/* Error Number for Filter Database */
#define RTL_EINVALIDFID					-1800 //Invalid Filter database ID
#define RTL_EFIDALREADYEXISTS			-1801 //Filter Database already exists.
#define RTL_EFIDISREFERENCEDBYVLAN		-1802 //Filter Database is being referenced by VLAN
#define RTL_EFIDISNOTEMPTY				-1803 //Filter Database is not empty
#define RTL_EINVALIDACTIONTYPE			-1804 //Invalid Port Action Type.
#define RTL_ENULLMACADDR					-1805 //The specified MAC address is NULL.
#define RTL_EL2ENTRYEXISTS				-1806 //The filter database entry already exists.
#define RTL_EL2ENTRYNOTFOUND				-1807 //The specified filter database entry was not found.
//#ifdef TEST_GETL2TBL
#define RTL_EGETL2_EMPTYL2TBL				-1808	/* The L2 table is empty */
#define RTL_EGETL2_INVALIDL2ENTRY			-1809	/* User specifies an invalid MAC address */
#define RTL_EGETL2_NOMOREVALIDL2ENTRY	-1810	/* No more valid L2 entry */
//#endif

/* Error Number for Aggregator */
#define RTL_EINVALIDAGGREGATORID			-1900 //Invalid aggregator ID.
#define RTL_EALREADYTRUNKING				-1901 //Aggregator already aggregates more than one port.
#define RTL_EAGGREGATORHASNOPORT			-1902 //Not allow aggregator without port member 
#define RTL_EAGGREGATOREXCEED			-1903 //The number of aggregator excees the system capability.
#define RTL_EAGGREGATORIDGTPORTNO		-1904 //Aggreagator ID > Port Number. When aggregating ports, 
												  //any port No. > aggregator ID is disallowed.
#define RTL_EAGGREGATORSETINDIVIDUAL		-1905 //Aggregator was set to individual, hence it can not 
												  //aggreate more then one port.
#define RTL_EDIFFBROADCASTDOMAIN			-1906 //Aggregator and port are in different broadcast domain.											  

/* Error Number for VLAN */
#define RTL_EVLANALREADYEXISTS			-2000 //Vlan already exists.
#define RTL_ECANNOTREMOVEDEFVLAN			-2001 //Cannot remove default VLAN.
#define RTL_EVLANISREFERENCEDBYNETIF		-2002 //Vlan is referenced by network interface.
#define RTL_EVLANISREFERENCEDBYPPPOE		-2003 //Vlan is referenced by PPPoE.
#define RTL_EVLANHASMACALLOCATED			-2004 //Vlan still has MAC address allocated.
#define RTL_EVLANHASHPORTMEMBER			-2005 //Vlan can not have member port while specifying its Filter database.
#define RTL_EVLANTXMIRRORSET				-2006 //Vlan has Tx mirror set hence can not set to promiscuous mode.
#define RTL_EPORTNOTVLANMEMBER			-2007 //The specified port is not a member of specified VLAN.
#define RTL_EVLANPROMISSET				-2008 //If VLAN promiscuous is set, a network interface cannot attach to the VLAN.
#define RTL_EFWDTXMIRRORSET				-2009 //Vlan Forwarding Tx Mirror set.
#define RTL_EVLANMACREFERENCEDBYPPPOE	-2010 //Vlan MAC is referenced by pppoe
#define RTL_EVLANMACREFERENCEDBYNETIF	-2011 //VLan MAC is referenced by network interface.
#define RTL_EINVVLANMAC					-2012 //Specified VLAN MAC number should be (0, 1, 2, 4, 8)
#define RTL_ENOUSABLEMAC					-2013 //No usable MAC address can be allocated
#define RTL_EVLANNOMAC					-2014 //Vlan has no MAC Address
#define RTL_ENOVLAN						-2015 //No more availiable VLAN entry



/* Error Number for NAT */
#define RTL_ENATIPNOTEXTERNALIP			-2100 //The specified NAT IP address is not an External IP address.
#define RTL_ENATIPNOTINTERNALIP			-2101 //The specified NAT Internal IP is not an Internal IP address.
#define RTL_EDUPLOCATENATIP				-2102 //The specified NAT IP address already maps to an Internal IP address.
#define RTL_ENATIPCANNOTBENAPTIP			-2103 //The specified NAT IP address cannot be a NAPT IP address.


/* Error Number for Routing */
#define RTL_ERTALREADYEXIST				-2300 //The routing entry already exists.
#define RTL_ERTINVALNEXTHOP				-2301 //Invalid nextHop
#define RTL_ENOSESSIONALLOCATE			-2302 //Need PPPoE session allocate
/*#ifdef TEST_GETRTTBL*/
#define RTL_ERTCREATEBYIF				-2303 //The route is added by IP interface creation. It can be removed.
#define RTL_EGETRT_EMPTYRTTBL			-2304	/* The routing table is empty */
#define RTL_EGETRT_NOMOREVALIDENTRY	-2305	/* No more valid routing entry */
#define RTL_EGETRT_INVALIDRTENTRY		-2306	/* User specifies an invalid route */
/*#endif */



/* Error Number for Arp */
#define RTL_ENONBROADCASTNET				-2400 //Non Boradcast network is forbidden to add ARP entry.
#define RTL_EARPALREADYEXIST				-2401 //Arp entry already exists.
#define RTL_EARPCANNOTADDL2ENTRY			-2402 //The L2 Entry cannot be added.
#define RTL_EARPCANNOTDELL2ENTRY			-2403 //The L2 Entry cannot be deleted.
#define RTL_ENOARPFOUND					-2404 //No ARP entry was found


/* Error Number for Network interface */
#define RTL_ENETIFREFERENCEDBYIPIF			-2500 //The network interface is referenced by IP interface.
#define RTL_ENETIFREFBYROUTE				-2501 //The network interface is referenced by a routing entry.
#define RTL_ELINKTYPESHOULDBERESET			-2502 //The link layer type should be reset before removing.
#define RTL_ENETIFREFERENCEDBYACL			-2503 //The network interface is referenced by ACL.
#define RTL_EUNKNOWLINKLAYERTYPE			-2504 //Unknow link layer type.
#define RTL_ECANNOTREMOVEIPUNNUMBER		-2505 //Can not remove IP unnumbered network itnerface. Use rtl8651_delIpUnnumbered() first.
#define RTL_ENETHASNOLLTYPESPECIFY			-2506 //The specified network interface has no link-layer type specified.
#define RTL_ENETISEXTERNAL					-2507 //The removed network interface is an external interface.
#define RTL_ENAPTSTART						-2508 //When NAPT starts, no manipulation about IP interface
												  //should be done
#define RTL_ENETIFALREADYEXTERNAL			-2509 //The specified network interface already is an external network interface.
#define RTL_ESHOULDBEEXTIF					-2510 //The network interface should be an external network interface
#define RTL_ESHOULDNOTDMZEXT				-2511 //The network interface should not be a DMZ external.
#define RTL_ELIPUNNUMBEREDNOTALLOW		-2512 //The specified network itnerface cannot be IP Unnumbered network.
#define RTL_ENETIFREFBYNATNAPT				-2513 //Network Interface is referenced BY NAT
#define RTL_ENETIFLLTYPEALREADYSET			-2514 //Link layer of the specified network itnerface type already set.
#define RTL_ENOLLTYPESPECIFY				-2515 //The network interface has no link layer type specified.


/* Error Number for IP Unnumbered */
#define RTL_EONLYONEIPUNNUMBERISALLOWED	-2600 //The whole system only allows one IP unnumbered network.
#define RTL_ENETIFINVALID				-2601 //One external and one internal network interfaces are needed.
#define RTL_EWANIFCANNOTHAVEIPINTF		-2602 //The external network interface cannot have IP interface.
#define RTL_NETIFTYPENOTMATCH			-2603 //The external network interface should be PPPoE type and internal interface should be VLAN type.
#define RTL_ENETIFNEEDIPUNNUMBERED		-2604 //The specified network itnerface should be IP unnumbered.
#define RTL_EIPUNNUMBEREDHASHIPINTF		-2605 //The IP Unnumebred still has external IP interface unnumebred attached.
#define RTL_ENETIFNOTIPUNNUMBERED		-2606 //The specified network interface is not an IP Unnumbered Network interface.
#define RTL_ENOTIPUNNUMBEREDNETIF	  	-2607 //The specified network interface is not IP Unnumbered Network interface.
#define RTL_EIPUNINTIPINTFNEED			-2608 //The specified network interface should be IP Unnumbered internal network interface.
#define RTL_EIPUNISREFERENCEDBYPPPOE		-2609 //The IP Unnumbered IP interface is referenced by PPPoE Session.
#define RTL_EEXTINTFSHOULDBEPPPOE		-2610 //The specified external network interface should be PPPoE type.
#define RTL_ESHOULDBEEXTNETIF			-2611 //The specified network interface should be external network interface.


/* Error Number for IP interface */
#define RTL_ENETMASKCANNOTBEZERO			-2701 //The netmask of IP interface cannot be zero.
#define RTL_EIPUNEXTIFCANNOTHASHIPINTF	-2702 //Cannot attach IP interface to External Network itnerface in an IP Unnumbered network.
#define RTL_EIPIFALREADYEXIST			-2703 //IP Interface already exists
#define RTL_EINVNETMASK					-2704 //Invalid network mask. The specified netmask may be 0xffffffff in a VLAN type network.
#define RTL_ECANNOTREMOVEIPUNEXTIPINTF	-2705 //Global IP interface of IP Unnumbered cannot be removed. Use rtl8651_setLanSideExternalIpInterface() first.
#define RTL_EIPINTFISREFERENCEDBYARP		-2706 //IP interface is referenced by ARP entry. It cannot be removed.
#define RTL_EIPINTFISREFERENCEDBYLS		-2707 //IP interface is referenced by Local Server. It cannot be removed.
#define RTL_EIPINTFISREFERENCEDBYRT		-2708 //IP interface is referenced by routing entry. It cannot be removed.
#define RTL_EIPINTFISREFERENCEDNYNATNAPT	-2709 //IP interface is referenced by NAT/NAPT. It cannot be removed.
#define RTL_ENOIPINTFFOUND				-2710 //The specified IP interface was not found.
#define RTL_ENOTEXTIP					-2711 //Specified external IP address doesn't belong to external interface
#define RTL_ENOTINTIP					-2712 //Specified internal IP address doesn't belong to internal interface

/* Error Number for PPPoE */
#define RTL_EFORBIDPROMISCINPPPOE		-2800 //If promiscuous mode was set, PPPoE cannot be created.
#define RTL_EVLANSHOULDBEPPPOE			-2801 //The specified VLAN should be PPPoE Type.
#define RTL_EPPPOEALREADYEXISTS			-2802 //The specified PPPoE already exists.
#define RTL_EOUTOFVLANMACADDR			-2803 //Out of VLAN MAC address.
#define RTL_EINVALIDPPPOEID				-2804 //Invalid PPPoE ID.
#define RTL_EPPPOEISINUSE				-2805 //The specified PPPoE is in use.
#define RTL_EPPPOEISREFERENCEDBYRT		-2806 //PPPoE is referenced by routing entry.
#define RTL_EINVALIDSESSIONID			-2807 //Invalid PPPoE Session ID.
#define RTL_EPROPERTUALREADYSET			-2808 //The specified PPPoE property was already set to another PPPoE ID.
#define RTL_EPPPOEHASPROPERTYSET			-2809 //The specified PPPoE ID already has property set.
#define RTL_EPPPOECANNOTADDL2ENTRY		-2810 //Cannot add L2 entry for the specified PPPoE.

/* Error Number for Local Server */
#define RTL_EINVALIDLSIPADDR				-2900 //Invalid Local Server IP address.
#define RTL_ELSCANNOTBEGWIP				-2901 //Local Server IP address cannot be gateway's IP address.
#define RTL_ELSALREADYEXISTS				-2902 //Local Server already exists.

/* Error Number for NAPT */
#define RTL_EDSTIPISLS					-3000 //NAPT DST IP is Local Server
#define RTL_EGETOFFSETFAIL				-3001 //NAPT get offset fail
#define RTL_EINVALIDEXTPORT				-3002 //Maybe the external port is in use or reserved
#define RTL_ENOTPERMIT					-3003 //The action is not allowed
#define RTL_EEXTIDINUSE					-3004 //NAPT ICMP external ID is in use
#define RTL_EDRVNAPTEXIST				-3005 //NAPT connection already exist in driver table
#define RTL_EINVL4PORTNUM				-3006 //Invalid UDP/TCP port number
#define RTL_EINVEXTIP					-3007 //Invliad external port number for NAPT
#define RTL_EDEFAULTROUTENOTFOUND		-3008 // Default Route is not defined.
#define RTL_EADDHASH2NEXTHOP				-3009 // Add Hash2 nexthop error
#define RTL_EDEFAULTNAPTIPEXIST			-3010 //Default Napt IP already exists
#define RTL_ENAPTCONNECTIONFULL		-3011	// napt system is full and can not add any other new flows
#define RTL_ENAPTNOTFOUNDWITHINFO		-3012 // Entry is not found, but return with infomation (for GetInbound()/Outbound())

/*Error Number for NAPTUSRMAPPING*/
#define RTL_EIPFILTEROVERLAPPING		-3050	//ipfilter overlap: the ip range and port is overloapping
#define RTL_ENAPTUSRMAPTYPEERROR		-3051	//napt user map entry's type is error
#define RTL_ENAPTUSRMAPFLAGERROR		-3052	//napt user map entry's flag is error
#define RTL_ENAPTUSRMAPFILTERNOTFOUND	-3053	// napt user map ipFilter not found
#define RTL_ENAPTUSRMAPDIRECTIONERROR	-3054	// napt usr map info's direction is error
#define RTL_ENAPTUSRMAPFILTERDROP		-3055	//pkt was drop by usr filter

/* Error Numebr for ALG */
#define RTL_ERANGEOVERLAP				-3100	//ALG port range overlap

/* Error Number Session: PPPoE, L2TP, PPTP */
#define RTL_ESESSIONNOTFOUND			-3200	//The specified Session ID is not found
#define RTL_ESESSIONREFERENCEDBYRT		-3201	//The specified Session is referenced by routing entry
#define RTL_ECANNOTMOVESESSION		-3202	//Can not move session to another network interface
#define RTL_EALREADYHAVESESSION		-3203	//The specified network interface already has session attached
#define RTL_ESESSIONISREFERENCED		-3204	//Session is referenced
#define RTL_ECANNOTUSETHISVID			-3205	//No more than one PPTP/L2TP VLAN is allowed

/* Error Number for Protocol-based NAT */
#define RTL_EPBNAT_ENTRY_EXIST			-3301 //The given {protocol,extip,intip} exists
#define RTL_EPBNAT_NO_FREE_ENTRY			-3302 //No free entry for Protocol-Based NAT
#define RTL_EPBNAT_ENTRY_NOT_FOUND		-3303 //The specific entry is not found
#define RTL_EPBNAT_PROTOCOL_NOT_SUPPORTED	-3304 //The specific protocol is not supported by protocol-based NAT


/* Error Number for Rate Limit */
#define RTL_EDUPGROUPID				-3401	//Duplicate Rate Limit Group ID
#define RTL_ERLENTRYISREFERENCE		-3402	//The specified rate limit entry is being referenced.
#define RTL_ENOGROUPIDFOUND			-3403	//The specified group ID was not found.
#define RTL_EEXCEEDTOTALBW			-3404	//The specified ratio exceeds total allowed bandwidth

/* Error Number for IPSec */
#define RTL_EIPSEC_PARAM_ERROR		-4001	//IPSec parameter error
#define RTL_EIPSEC_SPI_EXISTED		-4002	//IPSec SPI has existed
#define RTL_EIPSEC_SPI_NOT_FOUNT		-4003	//IPSec SPI not found
#define RTL_EIPSEC_NO_MORE_SPI		-4004	//no more availiable IPSec SPI 
#define RTL_EIPSEC_SPIGRP_EXISTED	-4007	//IPSec SPIGRP has existed
#define RTL_EIPSEC_SPIGRP_NOT_FOUNT	-4008	//IPSec SPIGRP not found
#define RTL_EIPSEC_NO_MORE_SPIGRP	-4009	//no more availiable IPSec SPIGRP
#define RTL_EIPSEC_EROUTE_EXISTED	-4012	//IPSec SPIGRP has existed
#define RTL_EIPSEC_EROUTE_NOT_FOUNT	-4013	//IPSec SPIGRP not found
#define RTL_EIPSEC_NO_MORE_EROUTE	-4014	//no more availiable IPSec SPIGRP
#define RTL_EIPSEC_INVALID_WINSIZE	-4015	//Invalid window size (See RFC2046 3.4.3)
#define RTL_EIPSEC_NO_MORE_NBT_ENTRY -4016 //no more netBios entry

/* Error Number for URL Filter */
#define RTL_EURLFILTER_URLSTRINGLEN_EXCEEND	-4501	// url filter string lenght exceeds system buffer size.
#define RTL_EURLFILTER_PATHSTRINGLEN_EXCEEND	-4502	// path string of url exceeds system buffer size
#define RTL_EMEMALLOCATEFAILEDFORURLFILTER	-4503	/* memory allocation failed for url filter operation */
/* RTL865XB_URLFILTER_UNKNOWNURLTYPE_SUPPORT */
#define RTL_EURLFILTER_INVALIDRULETYPE			-4504	/* Invalid url rule type */
/* RTL865XB_URLFILTER_ACTIONTYPE_SUPPORT */
#define RTL_EURLFILTER_INVALIDACTIONTYPE		-4505	/* Invalid action type of url filter rule */
/* RTL865XB_WEB_CONTENT_HDR_FILTER */
#define RTL_EURLFILTER_CONTENTSTRINGLEN_EXCEEND 	-4506	/* content string pattern exceeds system buffer size */
#define RTL_EURLFILTER_HTTPHDRSTRINGLEN_EXCEEND 	-4507	/* http header string pattern exceeds system buffer size */
#define RTL_EURLFILTER_CONTENTHDRFILTERINUSED		-4508	/* the content/httpHdr filter is used by an napt flow, cannot be deleted */

/* Error Number for PCM drivers */
#define RTL_EPCM_QUEUE_UNAV    -5001		/* pcm queue not allocated */
#define RTL_EPCM_PGAE_UNAV     -5002		/* pcm page not allocated */
#define RTL_EPCM_QUEUE_FULL    -5003		/* pcm queue is full of data */
#define RTL_EPCM_QUEUE_EMPTY   -5004		/* pcm queue is no   of data */
#define RTL_EPCM_QUEUE_SUBSIZE -5005		/* pcm queue has not enough data */
#define RTL_EPCM_BUF_UNAV      -5006		/* requested buffer not allocated */
#define RTL_EPCM_CHANNEL_NULL  -5007     /* current channel is not allocated */

/*Error number for naptUsrMapping*/
#define RTL_EVLANRANGETOOSMALL       -5100   /* set multiple pppoe id range too small RTL8651_IDLETIMEOUT_FIXED */
//#define RTL_NAPTUSRMAP_

/*Error number for reference count*/
#define RTL_EREFERENCEDBYOTHER	-5200


#endif
