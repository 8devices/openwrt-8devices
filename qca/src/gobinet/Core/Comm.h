/*===========================================================================
FILE:
   Comm.h

DESCRIPTION:
   Declaration of cComm class

PUBLIC CLASSES AND METHODS:
   cComm
      This class wraps low level port communications

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
#include "Event.h"
#include "Connection.h"

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

/*=========================================================================*/
// Class cComm
/*=========================================================================*/
class cComm : public cConnection
{
   public:
      // Constructor
      cComm();

      // Destructor
      ~cComm();

      // Is this object valid?
      bool IsValid();

      // Connect to the specified port
      bool Connect( LPCSTR pPort );
      
      // Run an IOCTL on the open file handle
      int SendCtl(
         UINT     ioctlReq,
         void *   pData );

      // Disconnect from the current port
      bool Disconnect();

      // Configure the port with the passed in parameters
      bool ConfigureSettings( termios * pSettings );

      // Return the current port settings
      bool GetSettings( termios * pSettings );

      // Cancel any in-progress I/O
      bool CancelIO();

      // Cancel any in-progress receive operation
      bool CancelRx();

      // Cancel any in-progress transmit operation
      bool CancelTx();

      // Receive data
      bool RxData(
         BYTE *                     pBuf, 
         ULONG                      bufSz,
         cIOCallback *              pCallback );
      
      // Transmit data
      bool TxData(
         const BYTE *               pBuf, 
         ULONG                      bufSz );

      // (Inline) Return current port name
      std::string GetPortName() const 
      { 
         return mPortName; 
      };

      // Are we currently connected to a port?
      bool IsConnected()
      {
         return (mPort != INVALID_HANDLE_VALUE);
      };

   protected:
      /* Name of current port */
      std::string mPortName;

      /* Handle to COM port */
      int mPort;
      
      // Cancel the write request?
      bool mbCancelWrite;

      /* Buffer */
      BYTE * mpBuffer;

      /* Buffer size */
      ULONG mBuffSz;

      /* Pipe for comunication with thread */
      int mCommandPipe[2];

      /* Thread ID of Rx Thread. */
      pthread_t mRxThreadID;

      // Rx thread is allowed complete access
      friend void * RxThread( void * pData );
};
