//------------------------------------------------------------------------------
// Copyright (c) 2011-2012 by European Organization for Nuclear Research (CERN)
// Author: Lukasz Janyst <ljanyst@cern.ch>
//------------------------------------------------------------------------------
// XRootD is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// XRootD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with XRootD.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#ifndef __XRD_CL_SOCKET_HH__
#define __XRD_CL_SOCKET_HH__

#include <stdint.h>
#include <string>
#include <sys/socket.h>

#include "XrdCl/XrdClStatus.hh"
#include "XrdNet/XrdNetAddr.hh"


namespace XrdCl
{
  class AnyObject;
  class Tls;
  class AsyncSocketHandler;

  //----------------------------------------------------------------------------
  //! A network socket
  //----------------------------------------------------------------------------
  class Socket
  {
    public:
      //------------------------------------------------------------------------
      //! Status of the socket
      //------------------------------------------------------------------------
      enum SocketStatus
      {
        Disconnected  = 1,      //!< The socket is disconnected
        Connected     = 2,      //!< The socket is connected
        Connecting    = 3       //!< The connection process is in progress
      };

      //------------------------------------------------------------------------
      //! Constructor
      //!
      //! @param socket already connected socket if available, -1 otherwise
      //! @param status status of a socket if available
      //------------------------------------------------------------------------
      Socket( int socket = -1, SocketStatus status = Disconnected ):
        pSocket(socket), pStatus( status ), pServerAddr( 0 ),
        pProtocolFamily( AF_INET ),
        pChannelID( 0 ),
        pCorked( false ),
        pTls( 0 )
      {
      };

      //------------------------------------------------------------------------
      //! Desctuctor
      //------------------------------------------------------------------------
      ~Socket();

      //------------------------------------------------------------------------
      //! Initialize the socket
      //------------------------------------------------------------------------
      Status Initialize( int family = AF_INET );

      //------------------------------------------------------------------------
      //! Set the socket flags (man fcntl)
      //------------------------------------------------------------------------
      Status SetFlags( int flags );

      //------------------------------------------------------------------------
      //! Get the socket flags (man fcntl)
      //------------------------------------------------------------------------
      Status GetFlags( int &flags );

      //------------------------------------------------------------------------
      //! Get socket options
      //------------------------------------------------------------------------
      Status GetSockOpt( int level, int optname, void *optval,
                         socklen_t *optlen );

      //------------------------------------------------------------------------
      //! Set socket options
      //------------------------------------------------------------------------
      Status SetSockOpt( int level, int optname, const void *optval,
                         socklen_t optlen );

      //------------------------------------------------------------------------
      //! Connect to the given host name
      //!
      //! @param host   name of the host to connect to
      //! @param port   port to connect to
      //! @param timout timeout in seconds, 0 for no timeout handling (may be
      //!               used for non blocking IO)
      //------------------------------------------------------------------------
      Status Connect( const std::string &host,
                      uint16_t           port,
                      uint16_t           timout = 10 );

      //------------------------------------------------------------------------
      //! Connect to the given host address
      //!
      //! @param addr   address of the host to connect to
      //! @param timout timeout in seconds, 0 for no timeout handling (may be
      //!               used for non blocking IO)
      //------------------------------------------------------------------------
      Status ConnectToAddress( const XrdNetAddr &addr,
                               uint16_t          timout = 10 );

      //------------------------------------------------------------------------
      //! Disconnect
      //------------------------------------------------------------------------
      void Close();

      //------------------------------------------------------------------------
      //! Get the socket status
      //------------------------------------------------------------------------
      SocketStatus GetStatus() const
      {
        return pStatus;
      }

      //------------------------------------------------------------------------
      //! Set socket status - do not use unless you know what you're doing
      //------------------------------------------------------------------------
      void SetStatus( SocketStatus status )
      {
        pStatus = status;
      }

      //------------------------------------------------------------------------
      //! Read raw bytes from the socket
      //!
      //! @param buffer    data to be sent
      //! @param size      size of the data buffer
      //! @param timeout   timout value in seconds, -1 to wait indefinitely
      //! @param bytesRead the amount of data actually read
      //------------------------------------------------------------------------
      Status ReadRaw( void *buffer, uint32_t size, int32_t timeout,
                      uint32_t &bytesRead );

      //------------------------------------------------------------------------
      //! Write raw bytes to the socket
      //!
      //! @param buffer       data to be written
      //! @param size         size of the data buffer
      //! @param timeout      timeout value in seconds, -1 to wait indefinitely
      //! @param bytesWritten the amount of data actually written
      //------------------------------------------------------------------------
      Status WriteRaw( void *buffer, uint32_t size, int32_t timeout,
                       uint32_t &bytesWritten );

      //------------------------------------------------------------------------
      //! Portable wrapper around SIGPIPE free send
      //!
      //! @param buffer : data to be written
      //! @param size   : size of the data buffer
      //! @return       : the amount of data actually written
      //------------------------------------------------------------------------
      Status Send( const char *buffer, size_t size, int &bytesWritten );

      //----------------------------------------------------------------------------
      //! Read helper for raw socket
      //!
      //! @param buffer    : the sink for the data
      //! @param size      : size of the sink
      //! @param bytesRead : number of bytes actually written into the sink
      //! @return          : success     : ( stOK )
      //!                    EAGAIN      : ( stOK,    suRetry )
      //!                    EWOULDBLOCK : ( stOK,    suRetry )
      //!                    other error : ( stError, errSocketError )
      //----------------------------------------------------------------------------
      Status Read( char *buffer, size_t size, int &bytesRead );

      //------------------------------------------------------------------------
      //! Get the file descriptor
      //------------------------------------------------------------------------
      int GetFD()
      {
        return pSocket;
      }

      //------------------------------------------------------------------------
      //! Get the name of the socket
      //------------------------------------------------------------------------
      std::string GetSockName() const;

      //------------------------------------------------------------------------
      //! Get the name of the remote peer
      //------------------------------------------------------------------------
      std::string GetPeerName() const;

      //------------------------------------------------------------------------
      //! Get the string representation of the socket
      //------------------------------------------------------------------------
      std::string GetName() const;

      //------------------------------------------------------------------------
      //! Get the server address
      //------------------------------------------------------------------------
      const XrdNetAddr &GetServerAddress() const
      {
        return pServerAddr;
      }

      //------------------------------------------------------------------------
      //! Set Channel ID
      //! (an object that allows to identify all sockets corresponding to the same channel)
      //------------------------------------------------------------------------
      void SetChannelID( AnyObject *channelID )
      {
        pChannelID = channelID;
      }

      //------------------------------------------------------------------------
      //! Get Channel ID
      //! (an object that allows to identify all sockets corresponding to the same channel)
      //------------------------------------------------------------------------
      const AnyObject* GetChannelID() const
      {
        return pChannelID;
      }

      //------------------------------------------------------------------------
      // Classify errno while reading/writing
      //------------------------------------------------------------------------
      static Status ClassifyErrno( int error );

      //------------------------------------------------------------------------
      // Cork the underlying socket
      //
      // As there is no way to do vector writes with SSL/TLS we need to cork
      // the socket and then flash it when appropriate
      //------------------------------------------------------------------------
      Status Cork();

      //------------------------------------------------------------------------
      // Uncork the underlying socket
      //------------------------------------------------------------------------
      Status Uncork();

      //------------------------------------------------------------------------
      // Flash the underlying socket
      //------------------------------------------------------------------------
      Status Flash();

      //------------------------------------------------------------------------
      // Check if the socket is corked
      //------------------------------------------------------------------------
      inline bool IsCorked() const
      {
        return pCorked;
      }

      //------------------------------------------------------------------------
      // Do special event mapping if applicable
      //------------------------------------------------------------------------
      uint8_t MapEvent( uint8_t event );

      //------------------------------------------------------------------------
      // Enable encryption
      //------------------------------------------------------------------------
      Status EnableEncryption( AsyncSocketHandler *socketHandler );

    protected:
      //------------------------------------------------------------------------
      //! Poll the socket to see whether it is ready for IO
      //!
      //! @param  readyForReading poll for readiness to read
      //! @param  readyForWriting poll for readiness to write
      //! @param  timeout         timeout in seconds, -1 to wait indefinitely
      //! @return stOK                  - ready for IO
      //!         errSocketDisconnected - on disconnection
      //!         errSocketError        - on socket error
      //!         errSocketTimeout      - on socket timeout
      //!         errInvalidOp          - when called on a non connected socket
      //------------------------------------------------------------------------
      Status Poll( bool readyForReading, bool readyForWriting,
                   int32_t timeout );

      int                  pSocket;
      SocketStatus         pStatus;
      XrdNetAddr           pServerAddr;
      mutable std::string  pSockName;     // mutable because it's for caching
      mutable std::string  pPeerName;
      mutable std::string  pName;
      int                  pProtocolFamily;
      AnyObject           *pChannelID;
      bool                 pCorked;

      Tls                 *pTls;
  };
}

#endif // __XRD_CL_SOCKET_HH__
 
