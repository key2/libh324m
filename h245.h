#ifndef _H245_H_
#define _H245_H_


int DestroyMultiplex(MxElement *elem, int isdestroyable);
void printh245(char *buf, int len);
size_t h245GetMessage(h324m_session *session, unsigned char* buf, unsigned int len, unsigned char *out);
size_t h245_MessageFromStateMachine(h324m_session *session, unsigned char* buf);
#endif // _H245_H_

