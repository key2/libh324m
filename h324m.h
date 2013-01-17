#ifndef _H324M_H_
#define _H324M_H_


#define PDU_LEN 160
#define SPOOL_MAX 1024

/*Log defines*/
#define H324_LOG_ERROR 1,__FILE__,__LINE__
#define H324_LOG_WARNING 2,__FILE__,__LINE__
#define H324_LOG_DEBUG 3,__FILE__,__LINE__

/*HDLC flags type*/
#define		HDLC_FLAG_S			"\x87\xb2"
#define		HDLC_FLAG_F			"\x78\x4d"
#define		HDLC_S	1
#define		HDLC_F	2


/*H245 State*/

#define STATE_TERMCAP						1
#define STATE_MASTERSLAVE_DETERMINATION		2
#define STATE_OLC_AUDIO						4
#define STATE_OLC_VIDEO						8
#define STATE_MULTIPLEX_ENTRY_SEND			16


#define SLAVE  1
#define MASTER 2



/* AL types */
#define		TYPE_AL1			1
#define		TYPE_AL2			2
#define		TYPE_AL3			4
#define		SegmentableFlag		8
#define		HasSeqNum			16


typedef struct MxElement { 
	int  RC; 
	struct MxElement **subelement; 
	int  subElementCount; 
	int  LCN; 
	char used; 
	struct MxElement *nextelement;
} MxElement;


typedef struct LCN {
	unsigned char *data;
	unsigned int len;
	unsigned char type;
	unsigned char SeqNum;
	int		(*ALTransport)(void *session, int lcn);
	int		(*ALUser)(void *session, unsigned char *buf, unsigned int len);

} LCN;


typedef struct H245Message {
	unsigned char *data;
	unsigned int len;
	unsigned char Acked;
} H245Message;

typedef struct NsrpReqAck {
	unsigned int CRC;
	unsigned char RepSN;
	unsigned char *Response;
	unsigned int RespLen;
}NsrpReqAck;


typedef struct SpoolBuffers {
	unsigned int nb;
	unsigned char locked;
	unsigned char *data[SPOOL_MAX];
	unsigned int len[SPOOL_MAX];
}	SpoolBuffers;


typedef struct h324m_session {
	unsigned char *raw_buffer;
	unsigned int  raw_buffer_len;
	unsigned char *pdu;
	unsigned int pdu_len;
	unsigned char tmpl;
	unsigned char tmc;
	unsigned int p;
	unsigned char AudioSN;
	unsigned char VideoSN;
	unsigned char Output[PDU_LEN];
	unsigned char Remaining[10];
	unsigned int RemainingLen;
	SpoolBuffers h245Spool;
	SpoolBuffers VideoSpool;
	SpoolBuffers AudioSpool;
	int flagtype;
	MxElement MultiPlexEntryTable[16];
	LCN	lcntable[256]; //dont need
	NsrpReqAck NsrpMsg[256];
	unsigned char StateMachine;
	unsigned int SessionTimer;
	H245Message h245msg[256];
	unsigned char NsrpSN;
	unsigned int TerminalType;
	unsigned int  StatusDeterminationNumber;
	unsigned char MasterSlave;
	unsigned char *videobuf;
	unsigned int videolen;
	unsigned char *videobuftmp;
	unsigned int videolentmp;

	unsigned char *audiobuf;
	unsigned char audiolen;
	unsigned char audiolock;
	unsigned char videolock;
	char DTMF[80];
}h324m_session;


int h324m_GetDTMF(h324m_session *session, char *dtmf_out);
void h324m_GetAudioBuffer(h324m_session *session, unsigned char *buf,unsigned int *len);
void h324m_GetVideoBuffer(h324m_session *session, unsigned char *buf,unsigned int *len);
int h324m_Amr(void *ss, unsigned char *buf, unsigned int len);
int h324m_h263(void *ss, unsigned char *buf, unsigned int len);
int h324m_263Show(void *ss, unsigned char *buf, unsigned int len);
int h324m_AmrShow(void *ss, unsigned char *buf, unsigned int len);
int h324m_TreatPackets(h324m_session *session);
int myaffiche(unsigned char *buf, int len);
int h324m_ExtractFirstPdu(h324m_session *session);
int h324m_StateMachine(h324m_session *session);
int h324m_MakePdu(h324m_session *session);
int h324m_SpoolH245(h324m_session *session, unsigned char *buf, unsigned int len);
int h324m_SpoolVideoBuffer(h324m_session *session, unsigned char *buf, unsigned int len);
int h324m_SpoolAudioBuffer(h324m_session *session, unsigned char *buf, unsigned int len);
unsigned char *reversebuf(unsigned char *c, unsigned int len);
int h324m_AL1(struct h324m_session *session, int lcn);
int h324m_AL2(void *ss, int lcn);
int h324m_ALNull(struct h324m_session *session, int lcn);
int h324m_NSRP(struct h324m_session *session, int lcn);
int h324m_Demultiplex(MxElement *a,h324m_session *session);
unsigned char reverse(unsigned char a);
int h324m_log(int errtype, char *file, unsigned int line, char *msg);
h324m_session *new_session();
int h324m_CloseSession(h324m_session *session);
int h324m_OnReceived(h324m_session *session, unsigned char *buf, unsigned int len);

#endif //_H324M_H_

