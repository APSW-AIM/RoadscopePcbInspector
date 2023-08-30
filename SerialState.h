#ifndef SERIALSTATE_H
#define SERIALSTATE_H


enum SerialState
{
    Disconnected,
    Connected,
    RxError,
    TxError,
    DoneBytesWritten,
    Timeout,
};


#endif // SERIALSTATE_H
