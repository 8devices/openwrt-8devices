/*===========================================================================
FILE:
   QMIProtocolServer.cpp

DESCRIPTION:
   QMI protocol server
   
PUBLIC CLASSES AND METHODS:
   cQMIProtocolServer

Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived from 
      this software without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "QMIProtocolServer.h"
#include "QMIBuffers.h"
#include "Comm.h"
#include "Socket.h"

/*=========================================================================*/
// cQMIProtocolServer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cQMIProtocolServer (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   serviceType [ I ] - QMI service type requested
   bufferSzRx  [ I ] - Size of data buffer for incoming data
   logSz       [ I ] - Size of log (number of buffers)

SEQUENCING:
   None (constructs sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cQMIProtocolServer::cQMIProtocolServer( 
   eQMIService                serviceType,
   ULONG                      bufferSzRx,
   ULONG                      logSz )
   :  cProtocolServer( MapQMIServiceToProtocol( serviceType, false ),
                       MapQMIServiceToProtocol( serviceType, true ),
                       bufferSzRx, 
                       logSz ),
      mLastTID( (WORD)INVALID_QMI_TRANSACTION_ID ),
      mService( serviceType )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cQMIProtocolServer (Public Method)

DESCRIPTION:
   Destructor

SEQUENCING:
   None (constructs sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cQMIProtocolServer::~cQMIProtocolServer()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the configured QMI service using the given QMI
   control file

PARAMETERS:
   pControlFile   [ I ] - QMI control file

SEQUENCING:
   This method is sequenced according to the command event, i.e. any
   other thread that needs to send a command to the protocol server 
   thread will block until this method completes

RETURN VALUE:
   bool
===========================================================================*/
bool cQMIProtocolServer::Connect( LPCSTR pControlFile )
{
   // Assume failure
   bool bRC = false;
   if (IsValid( mService ) == false || mService == eQMI_SVC_CONTROL)
   {
      return bRC;
   }

   std::string name = pControlFile;
   if (name.find( "qcqmi" ) != std::string::npos)
   {
      mpConnection = new cComm();
      mConnectionType = eConnectionType_RmNet;
   }
   else
   {
      // SMD
      std::string defaultPath = "/var/";
      
      // Such as "SOCKET:/dev/socket/,0"
      if (name.find( "SOCKET:" ) == 0)
      {
         // Remove qualifier
         name = name.substr( 7 );
      }
      // Such as "QMUXD:0"
      else if (name.find( "QMUXD:" ) == 0)
      {
         name = name.substr( 6 );
         name = defaultPath + "," + name;
      }
      // Just channel ID, IE: "0"
      else
      {
         name = defaultPath + "," + name;
      }

      mpConnection = new cSocket();
      mConnectionType = eConnectionType_SMD;
   }

   // Pass service file to base class for actual connection
   bRC = cProtocolServer::Connect( name.c_str() );
   
   if (bRC == false)
   {
      TRACE( "QMI connect %d failed\n", mService );
   }
   
   return bRC;
}

/*===========================================================================
METHOD:
   ValidateRequest (Internal Method)

DESCRIPTION:
   Validate a request that is about to be scheduled

SEQUENCING:
   This method is sequenced according to the command event, i.e. any
   other thread that needs to send a command to the protocol server 
   thread will block until this method completes

RETURN VALUE:
   bool
===========================================================================*/
bool cQMIProtocolServer::ValidateRequest( const sProtocolRequest & req )
{
   if (cProtocolServer::ValidateRequest( req ) == false)
   {
      return false;
   }

   sQMIServiceBuffer qmiReq( req.GetSharedBuffer() );
   return qmiReq.IsValid();
}

/*===========================================================================
METHOD:
   InitializeComm (Internal Method)

DESCRIPTION:
   Perform protocol specific communications port initialization

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cQMIProtocolServer::InitializeComm()
{
   bool bResult = false;
   if (mpConnection == 0)
   {
      return bResult;
   }

   if (mConnectionType == eConnectionType_RmNet)
   {
      // Setup the QMI Service type
      int result = mpConnection->SendCtl( QMI_GET_SERVICE_FILE_IOCTL, 
                                          (void*)(unsigned long)mService );
      bResult = (result == 0);
   }
   else if (mConnectionType == eConnectionType_SMD)
   {
      // Setup the QMI Service type
      int result = mpConnection->SendCtl( eQMUXD_MSG_ALLOC_QMI_CLIENT_ID, 
                                          (void*)&mService );
      bResult = (result == 0);
   }

   return bResult;
}

/*===========================================================================
METHOD:
   CleanupComm (Internal Method)

DESCRIPTION:
   Perform protocol specific communications port cleanup

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cQMIProtocolServer::CleanupComm()
{
   bool bResult = true;

   if (mpConnection != 0 
   && mConnectionType == eConnectionType_SMD)
   {
      // Delete the QMI client

      // NOTE: This is generally unnecessary, when you close the channel 
      // it is deleted
      int result = mpConnection->SendCtl( eQMUXD_MSG_RELEASE_QMI_CLIENT_ID, 
                                          0 );
      bResult = (result == 0);
   }

   // Nothing to actually do here
   return true;
}

/*===========================================================================
METHOD:
   DecodeRxData (Internal Method)

DESCRIPTION:
   Decode incoming data into QMI indications/responses

PARAMETERS:
   bytesReceived  [ I ] - Number of bytes to decoded
   rspIdx         [ O ] - Log index of last valid response (not used)
   bAbortTx       [ O ] - Response aborts current transmission? (not used)

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool - Was a response received?
===========================================================================*/
bool cQMIProtocolServer::DecodeRxData( 
   ULONG                      bytesReceived,
   ULONG &                    rspIdx,
   bool &                     bAbortTx )
{
   // Assume failure
   bool bRC = false;

   rspIdx = INVALID_LOG_INDEX;
   bAbortTx = false;

   // Something to decode from?
   if (bytesReceived == 0)
   {
      return bRC;
   }

   // Set protocol type (we have to be dealing with a valid QMI service)
   eProtocolType pt = MapQMIServiceToProtocol( mService, false );
   if (pt == ePROTOCOL_ENUM_BEGIN)
   {
      return bRC;
   }

   sSharedBuffer * pTmp = 0;
   pTmp = new sSharedBuffer( mpRxBuffer, bytesReceived, pt );
   if (pTmp != 0)
   {
      sQMIServiceBuffer tmpBuf( pTmp );
      if (tmpBuf.IsValid() == true)
      {
         rspIdx = mLog.AddBuffer( tmpBuf );
         if (IsResponse( tmpBuf ) == true)
         {
            bRC = true;
         }
         else
         {
            rspIdx = INVALID_LOG_INDEX;
         }
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   EncodeTxData (Internal Method)

DESCRIPTION:
   Encode data for transmission

PARAMETERS:
   pBuffer        [ I ] - Data to be encoded
   bEncoded       [ O ] - Do we even encode data?

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   sSharedBuffer * - Encoded data (0 upon error when encoding is indicated)
===========================================================================*/
sSharedBuffer * cQMIProtocolServer::EncodeTxData( 
   sSharedBuffer *            pBuffer,
   bool &                     bEncoded )
{
   WORD tid = ++mLastTID;
   if (tid == (WORD)INVALID_QMI_TRANSACTION_ID)
   {
      tid++;
   }

   sQMIServiceBuffer tmpBuf( pBuffer );
   tmpBuf.SetTransactionID( tid );
   
   // No actual encoding required as we alter the original request
   bEncoded = false;
   return 0;
};

/*===========================================================================
METHOD:
   IsResponse (Internal Method)

DESCRIPTION:
   Is the passed in data a response to the current request?

PARAMETERS:
   rsp         [ I ] - Candidate response

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cQMIProtocolServer::IsResponse( const sProtocolBuffer & rsp )
{
   // Assume not
   bool bRC = false;
   if ( (mpActiveRequest == 0) 
   ||   (mpActiveRequest->mRequest.IsValid() == false)
   ||   (mpActiveRequest->mbWaitingForResponse == false)
   ||   (rsp.IsValid() == false) )
   {
      return bRC;
   }

   sQMIServiceBuffer qmiReq( mpActiveRequest->mRequest.GetSharedBuffer() );
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );

   if (qmiReq.IsValid() == false || qmiRsp.IsValid() == false)
   {
      return bRC;
   }

   if (qmiRsp.IsResponse() == false)
   {
      return bRC;
   }

   WORD reqID = qmiReq.GetTransactionID();
   WORD rspID = qmiRsp.GetTransactionID();

   if ( (reqID == (WORD)INVALID_QMI_TRANSACTION_ID)
   ||   (rspID == (WORD)INVALID_QMI_TRANSACTION_ID)
   ||   (reqID != rspID) )
   {
      return bRC;
   }

   // Sadly there are documentated cases of firmware returning responses
   // with a matching transaction ID but a mismatching message ID.  There 
   // is no reason for this to be considered valid behavior as of yet
   ULONG reqMsgID = qmiReq.GetMessageID();
   ULONG rspMsgID = qmiRsp.GetMessageID();

   if (reqMsgID != rspMsgID)
   {
      return bRC;
   }

   bRC = true;
   return bRC; 
}
