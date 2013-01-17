#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "h324m.h"
#include "golay.h"
#include "crc8.h"
#include "crc16.h"
#include "h245.h"


int myaffiche(unsigned char *buf, int len)
{
	int i,m=0;
	for(i = 0; i <len; i++){
		//printf("%02x ",reverse(buf[i]));
		printf("%02x ",buf[i]);
		if (m == 15){
			printf("\n");
			m = 0;
		}
		else m++;
	}
	if(len)
		printf("\n");
	return 0;
}




unsigned char reverse(unsigned char a)
{
	unsigned char b;
	int i;
	b = 0;
	for (i =0; i < 8; i++){
		b = b << 1;
		b = b | (a & 1);
		a = a >> 1;
	}
	return b;
}






unsigned char *reversebuf(unsigned char *c, unsigned int len)
{
	unsigned char *tmp;
	unsigned int i;
	tmp = (unsigned char *)malloc(len * sizeof(unsigned char));
	if(!tmp){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}
	for(i = 0; i< len; i++){
		tmp[i] = reverse(c[i]);
	}
	return tmp;

}




int h324m_log(int errtype, char *file, unsigned int line, char *msg)
{
	printf("[%d] %s:%d %s",errtype,file,line,msg);
	return 0;
}




h324m_session *new_session(){
	int i;
	h324m_session *session;
	session = (h324m_session*)malloc(sizeof(h324m_session));
	if(!session){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return NULL;
	}
	session->raw_buffer = NULL;
	session->raw_buffer_len = 0;
	session->pdu = NULL;
	session->pdu_len = 0;
	for(i = 0; i < 16; i++){
		session->MultiPlexEntryTable[i].nextelement  = NULL;
		session->MultiPlexEntryTable[i].subelement   = NULL;
		session->MultiPlexEntryTable[i].RC = -1;
		session->MultiPlexEntryTable[i].LCN = i;
		session->MultiPlexEntryTable[i].subElementCount = 0;
	}
	for(i = 1; i< 256; i++){
		session->lcntable[i].data = NULL;
		session->lcntable[i].type = 0;
		session->lcntable[i].len = 0;
		session->lcntable[i].ALTransport = (void*)h324m_ALNull;
		session->lcntable[i].SeqNum = 0;
		session->lcntable[i].ALUser = NULL;
	}
	for(i = 0; i < 256; i++){
		session->NsrpMsg[i].CRC = 0;
		session->NsrpMsg[i].RepSN = 0;
		session->NsrpMsg[i].RespLen = 0;
		session->NsrpMsg[i].Response = NULL;
	}
	for(i = 0; i < SPOOL_MAX; i++){
		session->AudioSpool.data[i] = NULL;
		session->VideoSpool.data[i] = NULL;
		session->h245Spool.data[i] = NULL;
		session->AudioSpool.len[i] = 0;
		session->VideoSpool.len[i] = 0;
		session->h245Spool.len[i] = 0;
	}

	for(i = 0; i < 256; i++){
		session->h245msg[i].Acked = 0;
		session->h245msg[i].data = NULL;
		session->h245msg[i].len = 0;
	}

	session->lcntable[0].data = NULL;
	session->lcntable[0].type = TYPE_AL1 | SegmentableFlag;
	session->lcntable[0].len = 0;
	session->lcntable[0].ALTransport = (void*)h324m_NSRP;
	session->lcntable[0].SeqNum = 0;
	session->lcntable[i].ALUser = NULL;
	session->AudioSpool.nb = 0;
	session->VideoSpool.nb = 0;
	session->h245Spool.nb = 0;
	session->AudioSpool.locked = 0;
	session->VideoSpool.locked = 0;
	session->h245Spool.locked = 0;
	session->RemainingLen = 0;
	session->AudioSN = 0;
	session->VideoSN = 0;
	session->NsrpSN = 0;
	session->TerminalType = 160;
	session->StatusDeterminationNumber = 16777214;
	session->videobuf = NULL;
	session->videobuftmp = NULL;
	session->audiobuf = NULL;
	session->videolen = 0;
	session->videolentmp = 0;
	session->audiolen = 0;

	session->SessionTimer = 0;
	session->StateMachine = 0;
	session->audiolock = 0;
	session->videolock = 0;
	memset(session->DTMF,0,sizeof(session->DTMF));
	return session;
}



int h324m_CloseSession(h324m_session *session)
{
	int i;

	if(session->raw_buffer)
	  free(session->raw_buffer);

	if(session->pdu)
	  free(session->pdu);

	for(i = 0; i < 16; i++){
	  DestroyMultiplex(&session->MultiPlexEntryTable[i],0);	
	}
	for(i = 0; i < 256; i++){
	  if(session->NsrpMsg[i].Response)
	    free(session->NsrpMsg[i].Response);  
	  if(session->h245msg[i].data )
	    free(session->h245msg[i].data);
	  if(session->lcntable[i].data)
	    free(session->lcntable[i].data);
	}
 
	for(i = 0; i < SPOOL_MAX; i++){
	  if(session->AudioSpool.data[i])
	    free(session->AudioSpool.data[i]);
	  if(session->VideoSpool.data[i])
	    free(session->VideoSpool.data[i]);
	  if(session->h245Spool.data[i])
	    free(session->h245Spool.data[i]);
	}

	if(session->audiobuf)
		free(session->audiobuf);
	if(session->videobuf)
		free(session->videobuf);
	free(session);
	return 0;
}





int h324m_OnReceived(h324m_session *session, unsigned char *buf, unsigned int len)
{
	session->raw_buffer = (unsigned char*)realloc(session->raw_buffer,session->raw_buffer_len + len);
	if(!session->raw_buffer){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}
	memcpy(session->raw_buffer + session->raw_buffer_len, buf, len);
	session->raw_buffer_len += len;
	session->SessionTimer += len;
	h324m_TreatPackets(session);
	return len;	
}






int h324m_ExtractFirstPdu(h324m_session *session)
{
	unsigned int i;
	unsigned int len;
	unsigned int header;
	unsigned char *p1 = NULL;
	unsigned char *tmp;
	unsigned char c[3];

	if(session->pdu){
		free(session->pdu);
		session->pdu = NULL;
		session->pdu_len = 0;
	}
	i = 0;
find:
	for(; i < session->raw_buffer_len - 1; i++){
		if(!memcmp(session->raw_buffer + i,HDLC_FLAG_S,2) || !memcmp(session->raw_buffer + i,HDLC_FLAG_F,2)){
			i += 2;
			p1 = session->raw_buffer + i;
			break;	
		}
	}

	/*make sure there is a golay header*/
	if(session->raw_buffer_len - i < 3)
		return 0;



	header = (((p1[0] << 16) | (p1[1] << 8)) | p1[2]);
	if (!errwt(header)){
		c[0] = reverse(p1[0]);
		c[1] = reverse(p1[1]);
		c[2] = reverse(p1[2]);
		session->tmc = c[0] & 0xF;
		session->tmpl = (c[0] >> 4) & 0xF;
		session->tmpl = session->tmpl | ((c[1] << 4) & 0xF0);
		session->p = c[2];
		session->p = (session->p << 4) & 0xFF0;
		session->p = session->p | ((c[1] >> 4) & 0xF);
	}
	else{
		printf("Wrong header !\n");
		goto find;
	}

	/*Make sure there are enough data left*/
	if(session->raw_buffer_len - i  < (unsigned int)session->tmpl + 5){
		return 0;
	}


	if(!memcmp(session->raw_buffer + i + 3 + session->tmpl,HDLC_FLAG_S,2)){
		session->flagtype = HDLC_S;
	}
	else {
		session->flagtype = HDLC_F;
	}

	len = session->tmpl;

	if (!len){
		tmp = (unsigned char*)malloc(session->raw_buffer_len - i -3 - len);
		if(!tmp){
			h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
			return 0;
		}

		memcpy(tmp,session->raw_buffer + i + 3 + len, session->raw_buffer_len - i  -3 - len);
		free(session->raw_buffer);
		session->raw_buffer = tmp;
		session->raw_buffer_len = session->raw_buffer_len - i - 3 - len;
		session->pdu = NULL;
		session->pdu_len = 0;
		return 1;
	}
	session->pdu = (unsigned char*)malloc(len * sizeof(char));
	if(!session->pdu){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}
	memcpy(session->pdu,p1 + 3,len);

	tmp = (unsigned char*)malloc(session->raw_buffer_len - i -3 - len);
	if(!tmp){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}

	memcpy(tmp,session->raw_buffer + i + 3 + len, session->raw_buffer_len - i  -3 - len);
	free(session->raw_buffer);
	session->raw_buffer = tmp;
	session->raw_buffer_len = session->raw_buffer_len - i - 3 - len;
	session->pdu_len = len;
	return 1;
}







int h324m_H245Machine(h324m_session *session)
{
	unsigned int i;
	unsigned char buf[1024];
	unsigned int len;

	int m = 0;

	if(session->SessionTimer <= 1600){
		return 0;

	}else {
		session->SessionTimer = 0;
	}

	/*Resend all the unsent*/
	for(i = 0; i < 256; i++){
		if(session->h245msg[i].data){
			m = 1;
			h324m_SpoolH245(session,session->h245msg[i].data,session->h245msg[i].len);
		}
	}



	/*If nothing to send*/
	if(!m){
		len = (unsigned int)h245_MessageFromStateMachine(session,buf);


		if(len){
		//	h324m_log(LOG_DEBUG,"Sending New packet on  SN: ");
		//	printf("%d\n",session->NsrpSN);
		//	printh245((char*)buf, len);

			for(i = 0; i < len; i++){
				buf[i] = reverse(buf[i]);
			}

			if(session->h245msg[session->NsrpSN].data){
				free(session->h245msg[session->NsrpSN].data);
			}
			session->h245msg[session->NsrpSN].data = (unsigned char*)malloc((len + 5) * sizeof(unsigned char));
			if(!session->h245msg[session->NsrpSN].data){
				h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
				return -1;
			}
			memcpy(session->h245msg[session->NsrpSN].data,"\x9f\x00\xff",3);
			session->h245msg[session->NsrpSN].data[1] = reverse(session->NsrpSN);
			memcpy(session->h245msg[session->NsrpSN].data + 3,buf,len);
			crcappend(session->h245msg[session->NsrpSN].data, len + 3);
			session->h245msg[session->NsrpSN].len = len + 5;
			session->h245msg[session->NsrpSN].Acked = 0;
			h324m_SpoolH245(session, session->h245msg[session->NsrpSN].data, session->h245msg[session->NsrpSN].len );
			session->NsrpSN++;
		}
	}
	return 0;
}




int h324m_NSRP(struct h324m_session *session, int lcn)
{
	unsigned char *buf;
	unsigned int seqnum;
	unsigned int msgcrc;
	unsigned int len;
	unsigned char tmp[1024];
	unsigned int i;


	if(!crcverify(session->lcntable[lcn].data, session->lcntable[lcn].len)){
		h324m_log(H324_LOG_WARNING,"Wrong CRC16 on NSRP packet\n");	
		return -1;
	}
	seqnum = reverse(session->lcntable[lcn].data[1]);
	if(session->lcntable[lcn].data[0] == 0x9f){
		/*its a NSRP command so first NSRP ack it !*/
		tmp[0] = 0xEF;
		tmp[1] = reverse(seqnum);
		crcappend(tmp,2);
		h324m_SpoolH245(session,tmp,4);

		msgcrc = (session->lcntable[lcn].data[ session->lcntable[lcn].len - 2 ] << 8) | session->lcntable[lcn].data[ session->lcntable[lcn].len - 1];
		if(session->NsrpMsg[seqnum].CRC != msgcrc){
			if(session->NsrpMsg[seqnum].Response){
				free(session->NsrpMsg[seqnum].Response);
			}
			session->NsrpMsg[seqnum].RepSN = 0;
			session->NsrpMsg[seqnum].RespLen = 0;
			session->NsrpMsg[seqnum].CRC = msgcrc;
			buf = reversebuf(session->lcntable[lcn].data + 3, session->lcntable[lcn].len -5);
			if(!buf){
				h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
				return -1;
			}
			h324m_log(H324_LOG_DEBUG,"Received H245 message from remote part: \n");		
			printh245((char*)buf, (int)session->lcntable[lcn].len -5);
			len = (unsigned int)h245GetMessage(session, buf, session->lcntable[lcn].len -5,tmp);
			free(buf);
			if(len){
				printf("\n\nSo we answer:\n");
			//	h324m_log(LOG_DEBUG,"Sending Response on SN: ");
			//	printf("%d\n",session->NsrpSN);

				printh245((char*)tmp, len);
				session->NsrpMsg[seqnum].Response = (unsigned char*)malloc((len + 5 ) * sizeof(unsigned char));
				for(i = 0; i < len; i++){
					session->NsrpMsg[seqnum].Response[i+3] = reverse(tmp[i]);
				}
				session->NsrpMsg[seqnum].Response[0] = 0x9f;
				session->NsrpMsg[seqnum].Response[1] = reverse(session->NsrpSN);
				session->NsrpMsg[seqnum].Response[2] = 0xff;
				crcappend(session->NsrpMsg[seqnum].Response, len+3);
				session->NsrpMsg[seqnum].RespLen = len + 5;
				session->NsrpMsg[seqnum].RepSN = session->NsrpSN;

				if(session->h245msg[session->NsrpMsg[seqnum].RepSN].data){
				  free(session->h245msg[session->NsrpMsg[seqnum].RepSN].data);
				  session->h245msg[session->NsrpMsg[seqnum].RepSN].data = 0;
				}

				//h324m_SpoolH245(session, session->NsrpMsg[seqnum].Response, session->NsrpMsg[seqnum].RespLen);
				session->h245msg[session->NsrpMsg[seqnum].RepSN].Acked = 0;
				session->h245msg[session->NsrpMsg[seqnum].RepSN].data = (unsigned char*)malloc(session->NsrpMsg[seqnum].RespLen * sizeof(unsigned char));
				memcpy(session->h245msg[session->NsrpMsg[seqnum].RepSN].data ,session->NsrpMsg[seqnum].Response,session->NsrpMsg[seqnum].RespLen);
				session->h245msg[session->NsrpMsg[seqnum].RepSN].len = session->NsrpMsg[seqnum].RespLen;

				session->NsrpSN++;
			}
		} else {
		//	h324m_log(LOG_DEBUG,"Resending packet SN: ");
		//	printf("%d\n",session->NsrpMsg[seqnum].RepSN);
		  if(session->h245msg[session->NsrpMsg[seqnum].RepSN].data){
		    free(session->h245msg[session->NsrpMsg[seqnum].RepSN].data);
		    session->h245msg[session->NsrpMsg[seqnum].RepSN].data = 0;
		  }
			session->h245msg[session->NsrpMsg[seqnum].RepSN].Acked = 0;
			session->h245msg[session->NsrpMsg[seqnum].RepSN].data = (unsigned char*)malloc(session->NsrpMsg[seqnum].RespLen * sizeof(unsigned char));
			memcpy(session->h245msg[session->NsrpMsg[seqnum].RepSN].data ,session->NsrpMsg[seqnum].Response,session->NsrpMsg[seqnum].RespLen);
			session->h245msg[session->NsrpMsg[seqnum].RepSN].len = session->NsrpMsg[seqnum].RespLen;
		}
	} else {
		//printf("============================> Received Ack for SN %d\n",seqnum);
		session->h245msg[seqnum].Acked = 1;

		if(session->h245msg[seqnum].data){
			free(session->h245msg[seqnum].data);
			session->h245msg[seqnum].data = NULL;
		}


		session->h245msg[seqnum].len = 0;
	}
	return 0;
}






int h324m_AL1(struct h324m_session *session, int lcn)
{
	printf("\n\nData_AL1 LCN%d, len:%d, type: %d, flag: %d\n",lcn,session->lcntable[lcn].len, session->lcntable[lcn].type, session->flagtype);		
	myaffiche(session->lcntable[lcn].data, session->lcntable[lcn].len);
	return 0;
}





int h324m_ALNull(struct h324m_session *session, int lcn)
{

	printf("\n\nData unkown LCN%d, len:%d, flag: %d\n",lcn,session->lcntable[lcn].len,  session->flagtype);		
	myaffiche(session->lcntable[lcn].data, session->lcntable[lcn].len);
	return 0;

}



int if2toamr(unsigned char *buf, unsigned char *output)
{
	static unsigned char amr_bits[16] = { 95, 103, 118, 134, 148, 159, 204, 244, 39, 43, 38, 37, 0, 0, 0, 0};
	static unsigned char amr_header[16] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 4};
	static unsigned char amr_stuff[16] = { 5, 5, 6, 6, 0, 5, 0, 0, 5, 1, 6, 7, 0, 0, 0, 4};
	static unsigned char amr_bytes[16] = {13, 14, 16, 18, 19, 21, 26, 31, 6, 6, 6, 6, 0, 0, 0, 1};

	unsigned int amrtype;
	unsigned int i;
	int len;
	amrtype = reverse(buf[0]) & 0xf;
	len = amr_bytes[amrtype] +1;
	for(i = 0; i < amr_bytes[amrtype]; i++){
		output[i+1] =  (buf[i] << 4) | (buf[i+1] >> 4);
	}
	output[0] = (amrtype << 3) | 0x4;
	return len;
}


int amrtoif2(unsigned char *buf, unsigned char *output)

{
  static unsigned char amr_bits[16] = { 95, 103, 118, 134, 148, 159, 204, 244, 39, 43, 38, 37, 0, 0, 0, 0};
  static unsigned char amr_header[16] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 4};
  static unsigned char amr_stuff[16] = { 5, 5, 6, 6, 0, 5, 0, 0, 5, 1, 6, 7, 0, 0, 0, 4};
  static unsigned char amr_bytes[16] = {13, 14, 16, 18, 19, 21, 26, 31, 6, 6, 6, 6, 0, 0, 0, 1};
  unsigned char amrtype;
  unsigned char temp[65];
  int bytes;
  int i;

  amrtype = buf[0] >> 3;
  bytes = amr_bytes[amrtype];
  for(i = 0; i < bytes + 1; i++){
    temp[i] = ( buf[i + 1] >> 4) | (buf[i] << 4);
  }
  temp[0] = (temp[0] & 0xf) | (reverse(amrtype) & 0xf0);
  memcpy(output,temp,bytes);
  return bytes;
}




int h324m_AmrShow(void *ss, unsigned char *buf, unsigned int len)
{
	struct h324m_session *session = ss;
	unsigned char tmp[64];
	unsigned int slen;
	slen = if2toamr(buf,tmp);
	myaffiche(tmp,slen);
	return 0;
}


int h324m_Amr(void *ss, unsigned char *buf, unsigned int len)
{
	struct h324m_session *session = ss;
	unsigned char *tmp;
	unsigned int slen;
	tmp = (unsigned char*)malloc(64);
	//memcpy(tmp,buf,len);
	
	slen = if2toamr(buf,tmp);
	while(session->audiolock);
	session->audiolock = 1;
	if(session->audiobuf)
		free(session->audiobuf);
	session->audiobuf = tmp;
	session->audiolen = slen;
	session->audiolock = 0;
	return 0;
}


int h324m_263Show(void *ss, unsigned char *buf, unsigned int len)
{
	struct h324m_session *session = ss;
	unsigned char *tmp;
	tmp = reversebuf(buf,len);
	myaffiche(tmp,len);
	free(tmp);
	return 0;
}


int h324m_h263(void *ss, unsigned char *buf, unsigned int len)
{
	struct h324m_session *session = ss;
	unsigned char *tmp;
	tmp = reversebuf(buf,len);
	while(session->videolock);
	session->videolock = 1;
	
	if(session->videobuf){
	  session->videobuf = (unsigned char*)realloc(session->videobuf,len + session->videolen);
	  memcpy(session->videobuf + session->videolen,tmp,len);
	  session->videolen += len;
	  free(tmp);
	} else {
	  session->videobuf = tmp;
	  session->videolen = len;
	}
	session->videolock = 0;
	return 0;
}


int h324m_AL2(void *ss, int lcn)
{
	struct h324m_session *session = ss;
	if(!crc8_verify(session->lcntable[lcn].data,session->lcntable[lcn].len)){
		h324m_log(H324_LOG_WARNING, "Wrong CRC on packet \n");
	} 
	if(session->lcntable[lcn].type & HasSeqNum){
	//	printf("\n\nData_AL2 LCN%d, len:%d, SeqNum:%d,  type: %d, flag: %d\n",lcn,session->lcntable[lcn].len, reverse(session->lcntable[lcn].data[0]), session->lcntable[lcn].type, session->flagtype);		
		session->lcntable[lcn].ALUser(session, session->lcntable[lcn].data + 1, session->lcntable[lcn].len - 2);
	} else {
	//	printf("\n\nData_AL2 LCN%d, len:%d, type: %d, flag: %d\n",lcn,session->lcntable[lcn].len, session->lcntable[lcn].type, session->flagtype);		
		session->lcntable[lcn].ALUser(session, session->lcntable[lcn].data, session->lcntable[lcn].len - 1);
	}
	return 0;
}





int ALProceed(int lcn, unsigned int len, h324m_session *session)
{
	unsigned char *tmp;
	//	printf("\n\nProceed with LCN%d, len %d, flagtype: %d\n",lcn,len,session->flagtype);
	//	myaffiche(session->pdu,len);

	session->lcntable[lcn].data = (unsigned char*)realloc(session->lcntable[lcn].data, session->lcntable[lcn].len + len);
	memcpy(session->lcntable[lcn].data + session->lcntable[lcn].len, session->pdu, len);
	session->lcntable[lcn].len += len;

	if(session->lcntable[lcn].type & SegmentableFlag){
		if(session->flagtype == HDLC_F){
			session->lcntable[lcn].ALTransport(session,lcn);
			/*Call here the layer*/
			free(session->lcntable[lcn].data);
			session->lcntable[lcn].data=NULL;
			session->lcntable[lcn].len = 0;
		}
	} else {
		session->lcntable[lcn].ALTransport(session,lcn);
		/*Call here the layer*/
		free(session->lcntable[lcn].data);
		session->lcntable[lcn].data=NULL;
		session->lcntable[lcn].len = 0;

	}

	if(len < session->pdu_len){
		tmp = (unsigned char*)malloc((session->pdu_len - len)*sizeof(unsigned char));
		memcpy(tmp,session->pdu + len, (session->pdu_len - len));
		free(session->pdu);
		session->pdu = tmp;
		session->pdu_len -= len;
	} else {
		free(session->pdu);
		session->pdu = NULL;
		session->pdu_len = 0;
	}
	return 0;
}








int h324m_Demultiplex(MxElement *a,h324m_session *session)
{
	int i,j;
	if ( a->subElementCount == 0){
		if (a->RC == -1){
			ALProceed(a->LCN,session->pdu_len , session);
		}
		else {
			if (session->pdu_len  >= (unsigned int)a->RC) /*If the RC is finite, then make sure we have enough bytes to send */
				ALProceed(a->LCN,a->RC, session);
			else
				ALProceed(a->LCN, session->pdu_len  , session); /*Otherwise, just send what's left*/
		}
		if ((a->nextelement) && (session->pdu_len)){
			h324m_Demultiplex(a->nextelement, session);
		}
		return 0;
	}
	for (j = 0; j < a->RC ; j++){
		for (i = 0; i < a->subElementCount; i++){	
			h324m_Demultiplex(a->subelement[i], session);
			if (!session->pdu_len)
				break;
		}
		if (!session->pdu_len)
			break;
	}
	if ((a->nextelement) && (session->pdu_len))
		h324m_Demultiplex(a->nextelement, session);
	return 0;
}



int h324m_SpoolAudioBuffer(h324m_session *session, unsigned char *buf, unsigned int len)
{

	unsigned int i;
	unsigned char tmpbuf[128];
	unsigned int tmplen;

	if(!(session->StateMachine & STATE_OLC_AUDIO) || !len){
	  return 0;
	}

	while(session->AudioSpool.locked);
	if(session->AudioSpool.data[session->AudioSpool.nb]){
		free(session->AudioSpool.data[session->AudioSpool.nb]); /*Make sure that we clean in case nb overflows !*/
		session->AudioSpool.data[session->AudioSpool.nb] = NULL;
	}
	tmplen = amrtoif2(buf,tmpbuf);
	session->AudioSpool.data[session->AudioSpool.nb] = (unsigned char*)malloc((tmplen + 2) * sizeof(unsigned char));
	if(!session->AudioSpool.data[session->AudioSpool.nb]){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}

	memcpy(session->AudioSpool.data[session->AudioSpool.nb] + 1, tmpbuf, tmplen);
	session->AudioSpool.data[session->AudioSpool.nb][0] = reverse(session->AudioSN++);
	session->AudioSpool.data[session->AudioSpool.nb][tmplen + 1] = crc8_calculate(session->AudioSpool.data[session->AudioSpool.nb],tmplen + 1);
	session->AudioSpool.len[session->AudioSpool.nb] = tmplen + 2;
	//printf("Spooled: %x with size %d\n",session->AudioSpool.data[session->AudioSpool.nb], session->AudioSpool.len[session->AudioSpool.nb]);
	//myaffiche(session->AudioSpool.data[session->AudioSpool.nb],session->AudioSpool.len[session->AudioSpool.nb]);
	if(session->AudioSpool.nb < SPOOL_MAX -1){
		session->AudioSpool.nb++;
	}else {
		h324m_log(H324_LOG_ERROR, "The Audio spooler got too big \n");
	}
	return 0;
}




int h324m_SpoolVideoBuffer(h324m_session *session, unsigned char *buf, unsigned int len)
{

  unsigned int i,j;
  unsigned char *pnt,*mark,*remainder;
  unsigned int val,tmplen;

  if(!(session->StateMachine & STATE_OLC_VIDEO) || !len){
    return 0;
  }
  while(session->VideoSpool.locked);
 
  if(session->videobuftmp){
    session->videobuftmp = (unsigned char*)realloc(session->videobuftmp,(session->videolentmp + len) * sizeof(unsigned char));
  } else {
    session->videobuftmp = (unsigned char*)malloc(len * sizeof(unsigned char));
  }
  memcpy(session->videobuftmp + session->videolentmp,buf,len);
  session->videolentmp += len;
 
  pnt = session->videobuftmp + 1;
  mark = session->videobuftmp;
  for(j = 0; j < session->videolentmp -3; j++){
    val = (pnt[0] << 16) + (pnt[1] << 8) + pnt[2];
    val = val & 0xFFFF80;
    if (val == 0x000080){
      tmplen = pnt-mark;
 
      /*******************/
      if(session->VideoSpool.data[session->VideoSpool.nb]){
	free(session->VideoSpool.data[session->VideoSpool.nb]); /*Make sure that we clean in case nb overflows !*/
	session->VideoSpool.data[session->VideoSpool.nb] = NULL;
      }
      session->VideoSpool.data[session->VideoSpool.nb] = (unsigned char*)malloc((tmplen + 2) * sizeof(unsigned char));
      if(!session->VideoSpool.data[session->VideoSpool.nb]){
	h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
	return 0;
      }
      for(i = 0; i < tmplen; i++){
	session->VideoSpool.data[session->VideoSpool.nb][i+1] = reverse(mark[i]);
      }
      session->VideoSpool.data[session->VideoSpool.nb][0] = reverse(session->VideoSN++);
      session->VideoSpool.data[session->VideoSpool.nb][tmplen + 1] = crc8_calculate(session->VideoSpool.data[session->VideoSpool.nb],tmplen + 1);
      session->VideoSpool.len[session->VideoSpool.nb] = tmplen + 2;
      //myaffiche(session->VideoSpool.data[session->VideoSpool.nb],session->VideoSpool.len[session->VideoSpool.nb]);
      if(session->VideoSpool.nb < SPOOL_MAX -1){
	session->VideoSpool.nb++;
      }else {
	h324m_log(H324_LOG_ERROR, "The Video spooler got too big \n");
      }
      /**************/
      mark = pnt;
    }
    pnt++;
  }
  remainder = (unsigned char*)malloc(session->videolentmp - (mark - session->videobuftmp));
  memcpy(remainder,mark,session->videolentmp - (mark - session->videobuftmp));
  session->videolentmp = session->videolentmp - (mark - session->videobuftmp);
  free(session->videobuftmp);
  session->videobuftmp = remainder;
  return 0;
}



/*

int h324m_SpoolVideoBuffer(h324m_session *session, unsigned char *buf, unsigned int len)
{

	unsigned int i;

	if(!(session->StateMachine & STATE_OLC_VIDEO) || !len){
	  return 0;
	}
	while(session->VideoSpool.locked);
	if(session->VideoSpool.data[session->VideoSpool.nb]){
		free(session->VideoSpool.data[session->VideoSpool.nb]); /*Make sure that we clean in case nb overflows !*/
/*		session->VideoSpool.data[session->VideoSpool.nb] = NULL;
	}
	session->VideoSpool.data[session->VideoSpool.nb] = (unsigned char*)malloc((len + 2) * sizeof(unsigned char));
	if(!session->VideoSpool.data[session->VideoSpool.nb]){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}
	for(i = 0; i < len; i++){
		session->VideoSpool.data[session->VideoSpool.nb][i+1] = reverse(buf[i]);
	}
	session->VideoSpool.data[session->VideoSpool.nb][0] = reverse(session->VideoSN++);
	session->VideoSpool.data[session->VideoSpool.nb][len + 1] = crc8_calculate(session->VideoSpool.data[session->VideoSpool.nb],len + 1);
	session->VideoSpool.len[session->VideoSpool.nb] = len + 2;
	printf("\nH263\n");
	myaffiche(session->VideoSpool.data[session->VideoSpool.nb],session->VideoSpool.len[session->VideoSpool.nb]);
	if(session->VideoSpool.nb < 254){
		session->VideoSpool.nb++;
	}else {
		h324m_log(H324_LOG_ERROR, "The Video spooler got too big \n");
	}
	return 0;
}

*/


int h324m_SpoolH245(h324m_session *session, unsigned char *buf, unsigned int len)
{

	if(!len){
	    return 0;
	}
	while(session->h245Spool.locked);
	if( session->h245Spool.data[session->h245Spool.nb]){
		free(session->h245Spool.data[session->h245Spool.nb]); /*Make sure that we clean in case nb overflows !*/
		session->h245Spool.data[session->h245Spool.nb] = NULL;
	}
	session->h245Spool.data[session->h245Spool.nb] = (unsigned char*)malloc(len * sizeof(unsigned char));
	if(!session->h245Spool.data[session->h245Spool.nb]){
		h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
		return 0;
	}
	memcpy(session->h245Spool.data[session->h245Spool.nb],buf,len);
	session->h245Spool.len[session->h245Spool.nb] = len;
	//myaffiche(session->h245Spool.data[session->h245Spool.nb],session->h245Spool.len[session->h245Spool.nb]);
	if(session->h245Spool.nb < SPOOL_MAX -1){
		session->h245Spool.nb++;
	} else {
		h324m_log(H324_LOG_ERROR, "The H245 spooler got too big \n");
	}
	return 0;
}




int h324m_MakePdu(h324m_session *session)
{
	unsigned char buf[170];
	unsigned char *tmp;
	unsigned int pos = 0;
	unsigned int sz = 0;
	unsigned int i;
	char cmc, cmpl;
	long temp, ret;

	//	printf("----------------------DEBUG-----------------------\n");

	if(session->RemainingLen){
		memcpy(buf,session->Remaining,session->RemainingLen);
		
		//	printf("Remaining:\n");
		//	myaffiche(session->Remaining,session->RemainingLen);
		pos += session->RemainingLen;
		session->RemainingLen = 0;
	}

	/*make sure there is at least one Audio packet if available*/
	if(session->AudioSpool.nb){
		session->AudioSpool.locked = 1;
		for(i = 0; i < SPOOL_MAX; i++){
			if(session->AudioSpool.data[i]){
			  //  printf("Audio:\n");
				cmc = reverse((unsigned char)1);
				cmpl = reverse((unsigned char)session->AudioSpool.len[i]);
				temp = (unsigned char)cmc;
				temp = (temp << 4) | (unsigned char)cmpl;
				ret = Gencode(temp);
				buf[pos] = (unsigned char)((ret >> 16) & 0xFF);
				buf[pos+1] = (unsigned char)((ret >> 8) & 0xFF);
				buf[pos+2] = (unsigned char)((ret & 0xFF));
				pos += 3;
				memcpy(buf + pos, session->AudioSpool.data[i],session->AudioSpool.len[i]);
				//myaffiche(buf + pos -3,session->AudioSpool.len[i] + 3);
				free(session->AudioSpool.data[i]);
				session->AudioSpool.data[i] = NULL;
				pos += session->AudioSpool.len[i];
				session->AudioSpool.len[i] = 0;
				if(!session->AudioSpool.data[i + 1]){
					session->AudioSpool.nb = 0;
				}
				memcpy(buf + pos,HDLC_FLAG_S,2);
				pos += 2;
				
				break;
			}
		}
		session->AudioSpool.locked = 0;
	}

	if(session->h245Spool.nb){
		session->h245Spool.locked = 1;
		for(i = 0; i < SPOOL_MAX; i++){
			if(session->h245Spool.data[i]){
				if(pos + session->h245Spool.len[i] <= PDU_LEN - 5){ /*there are room remaining*/
				  //printf("H245\n");
					cmc = reverse((unsigned char)0);
					cmpl = reverse((unsigned char)session->h245Spool.len[i]);
					temp = (unsigned char)cmc;
					temp = (temp << 4) | (unsigned char)cmpl;
					ret = Gencode(temp);
					buf[pos] = (unsigned char)((ret >> 16) & 0xFF);
					buf[pos+1] = (unsigned char)((ret >> 8) & 0xFF);
					buf[pos+2] = (unsigned char)((ret & 0xFF));
					pos += 3;				
					memcpy(buf + pos,session->h245Spool.data[i],session->h245Spool.len[i]);
					//myaffiche(buf + pos - 3, session->h245Spool.len[i] + 3);
					free(session->h245Spool.data[i]);
					session->h245Spool.data[i] = NULL;
					pos += session->h245Spool.len[i];
					session->h245Spool.len[i] = 0;
					memcpy(buf + pos,HDLC_FLAG_F,2);
					pos += 2;
					if(!session->h245Spool.data[i + 1]){ 
						session->h245Spool.nb = 0;
						break;
					}
					memcpy(buf + pos,HDLC_FLAG_F,2);
					pos += 2;
				} else {
					if(pos >= PDU_LEN - 5){
						break;
					}
					//printf("H245:\n");
					sz = PDU_LEN - 5 - pos;
					tmp = (unsigned char*)malloc((session->h245Spool.len[i] - sz) * sizeof(unsigned char));
					if(!tmp){
						h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
						return 0;
					}
					cmc = reverse((unsigned char)0);
					//cmpl = reverse((unsigned char)session->h245Spool.len[i]);
					cmpl = reverse((unsigned char)sz);
					temp = (unsigned char)cmc;
					temp = (temp << 4) | (unsigned char)cmpl;
					ret = Gencode(temp);
					buf[pos] = (unsigned char)((ret >> 16) & 0xFF);
					buf[pos+1] = (unsigned char)((ret >> 8) & 0xFF);
					buf[pos+2] = (unsigned char)((ret & 0xFF));
					pos += 3;				
					memcpy(buf + pos,session->h245Spool.data[i],sz);
					//myaffiche(buf + pos - 3, sz + 3);
					memcpy(tmp,session->h245Spool.data[i] + sz, (session->h245Spool.len[i] - sz));
					pos += sz;
					free(session->h245Spool.data[i]);
					session->h245Spool.data[i] = tmp;
					session->h245Spool.len[i] -=  sz;
					memcpy(buf + pos,HDLC_FLAG_S,2);
					pos += 2;
					break;
				}
			}
		}
		session->h245Spool.locked = 0;
	}

	if(session->VideoSpool.nb){
		session->VideoSpool.locked = 1;
		for(i = 0; i < SPOOL_MAX; i++){
			if(session->VideoSpool.data[i]){
				if(pos + session->VideoSpool.len[i] < PDU_LEN - 5){ /*there are room remaining*/
				  //	  printf("Video:\n");
					cmc = reverse((unsigned char)2);
					cmpl = reverse((unsigned char)session->VideoSpool.len[i]);
					temp = (unsigned char)cmc;
					temp = (temp << 4) | (unsigned char)cmpl;
					ret = Gencode(temp);
					buf[pos] = (unsigned char)((ret >> 16) & 0xFF);
					buf[pos+1] = (unsigned char)((ret >> 8) & 0xFF);
					buf[pos+2] = (unsigned char)((ret & 0xFF));
					pos += 3;				
					memcpy(buf + pos,session->VideoSpool.data[i],session->VideoSpool.len[i]);
					//	myaffiche(buf + pos - 3, session->VideoSpool.len[i] + 3);
					free(session->VideoSpool.data[i]);
					session->VideoSpool.data[i] = NULL;
					pos += session->VideoSpool.len[i];
					session->VideoSpool.len[i] = 0;
					memcpy(buf + pos,HDLC_FLAG_F,2);
					pos += 2;
					if(!session->VideoSpool.data[i + 1]){ 
						session->VideoSpool.nb = 0;
						break;
					}
				} else {
					if(pos >= PDU_LEN - 5){
						break;
					}
					//printf("V");
					sz = PDU_LEN - 5 - pos;
					tmp = (unsigned char*)malloc((session->VideoSpool.len[i] - sz) * sizeof(unsigned char));
					if(!tmp){
						h324m_log(H324_LOG_ERROR, "Not enough memory !\n");
						return 0;
					}
					cmc = reverse((unsigned char)2);
					//cmpl = reverse((unsigned char)session->VideoSpool.len[i]);
					cmpl = reverse((unsigned char)sz);
					temp = (unsigned char)cmc;
					temp = (temp << 4) | (unsigned char)cmpl;
					ret = Gencode(temp);
					buf[pos] = (unsigned char)((ret >> 16) & 0xFF);
					buf[pos+1] = (unsigned char)((ret >> 8) & 0xFF);
					buf[pos+2] = (unsigned char)((ret & 0xFF));
					pos += 3;				
					memcpy(buf + pos,session->VideoSpool.data[i],sz);
					//	myaffiche(buf + pos - 3, sz + 3);
					memcpy(tmp,session->VideoSpool.data[i] + sz, (session->VideoSpool.len[i] - sz));
					pos += sz;
					free(session->VideoSpool.data[i]);
					session->VideoSpool.data[i] = tmp;
					session->VideoSpool.len[i] -=  sz;
					memcpy(buf + pos,HDLC_FLAG_S,2);
					pos += 2;
					break;
				}
			}
		}
		session->VideoSpool.locked = 0;
	}

	while(pos < PDU_LEN){
		memcpy(buf + pos,"\x00\x00\x00\x87\xb2",5);
		pos += 5;
	}

	if( pos > PDU_LEN){
		memcpy(session->Remaining,buf + PDU_LEN, pos - PDU_LEN);
		session->RemainingLen = pos - PDU_LEN;

	}
	memcpy(session->Output,buf,PDU_LEN);
	//	printf("Finalbuf:\n");
	//myaffiche(session->Output,PDU_LEN);
	//printf("\n\n\n\n\n");
	return 0;
}




int h324m_TreatPackets(h324m_session *session)
{
	while(h324m_ExtractFirstPdu(session)){
		if(session->tmpl)
			h324m_Demultiplex(&session->MultiPlexEntryTable[session->tmc], session);
	}
	h324m_H245Machine(session);
	return 0;
}


void h324m_GetAudioBuffer(h324m_session *session, unsigned char *buf,unsigned int *len)
{
	while(session->audiolock);
	session->audiolock = 1;
	memcpy(buf,session->audiobuf,session->audiolen);
	*len = session->audiolen;
	session->audiolock = 0;
}


void h324m_GetVideoBuffer(h324m_session *session, unsigned char *buf,unsigned int *len)
{
  if (!session->videolen){
    *len = 0;
    return;
  }
  while(session->videolock);
  session->videolock = 1;
  memcpy(buf,session->videobuf, session->videolen);
  *len = session->videolen;
  free(session->videobuf);
  session->videobuf = NULL;
  session->videolen = 0;
  session->videolock = 0;
  
}

int h324m_GetDTMF(h324m_session *session, char *dtmf_out)
{
	int ret;
	ret = strlen(session->DTMF);
	if(ret){
		memcpy(dtmf_out, session->DTMF, ret);
	}
	memset(session->DTMF,0,sizeof(session->DTMF));
	return ret;
}
