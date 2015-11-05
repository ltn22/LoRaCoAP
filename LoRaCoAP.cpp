#include <LoRaCoAP.h>

#ifdef DUMP_COAP
char *TypeMsg[] = { "CON", "NON", "ACK", "RST" };
char *TypeCode[] = { "PING", "GET", "POST", "PUT", "DELETE" };
#endif


// to send a packet we store in an Arduino buffer before doing
// a lora.write. May be it is possible to send byte per byto to the 
// the lora shield to avoid buffering.
byte outgoingBuf[50];   // 50 is the max size, NO CHECK!!!!!
int  idxOB = 0;        


CoAPServer::CoAPServer()
{
  resList = new CoAPResource ();

  s_mid = 1;
}

void CoAPServer::begin(LoraShield l, String n)
{
  l.init();
  Serial.println("Lorashield initailized");
  l.begin(n);
  Serial.print ("Registering "); Serial.println(n);  

  lora = l;
}

uint8_t CoAPServer::processRequest ()
{

  String URIPath = "";
  String input = ""; // data in request
  int option = 0, format = 0;
  uint16_t currentDelta;
  byte b;
  uint8_t status = 0xFF;
  uint8_t type;
  int tkl;
  uint8_t code;
  uint16_t mid;
  int cla, detail; 
  byte tok[8];
  CoAPResource *URIres;
  CoAPToken token;

  b = readByte ();  // first byte

  int version = (b & 0xC0) >> 6;

  if (version != 1) { // not CoAP skip
#ifdef DUMP_COAP
    Serial.println ("not a CoAP v1 msg");
#endif 
    status = 0xFE; 
    goto not_coap_err;
   }

  type = (b & 0x30) >> 4;
  tkl = (b & 0x0F);

  b = readByte (); // second byte

  code = b;

  mid = (readByte () << 8) | readByte (); // MID


#ifdef DUMP_COAP
  Serial.print ("ver:");
  Serial.print (version);
  Serial.print (" Type = ");
  Serial.print (type);
  Serial.print (" (");
  Serial.print ((type < 4) ? TypeMsg[type] : "???");
  Serial.print (")");
  Serial.print (" Token Length = ");
  Serial.print (tkl);
  Serial.print (" code ");
  Serial.print (code);
#endif
    cla = (code & 0xE0) >> 5;
    detail = code & 0x1F;

#ifdef DUMP_COAP
  Serial.print (" (");
  if (!cla)
    {
      Serial.print ((type < 4) ? TypeCode[detail] : "???");
    }
  else
    {
      Serial.print (cla);
      Serial.print (".");
      Serial.print (detail);
    }
  Serial.print (")");
  Serial.print (" Msg id = ");
  Serial.println (mid, HEX);
#endif

 
  for (int k=0; k <tkl; k++) 
    tok[k] = readByte();
  
  token.setToken (tkl, tok);

#ifdef DUMP_COAP
  Serial.print ("Token: ");
  token.printToken ();
#endif
  // process Options

  b = 0x00;

  while (lora.dataAvailable() && (b != 0xFF))
    {
      b = readByte ();
      if (b != 0xFF)
	{
	  uint32_t delta = (b & 0xF0) >> 4;
	  uint32_t length = (b & 0x0F);

	  if (delta == 0x0F)
	    {
	      if (length = 0x0F)
		break;
	      else
		{
		  status = ERROR_CODE (4, 02);
		  goto proc_error;
		};
	    }

	  if ((delta == 13) || (delta == 14))
	    {
	      uint16_t
		d1 = readByte () + 13;
	      if (delta == 14)
		d1 = d1 * 256 + readByte () + 256;

	      delta = d1;
	    }

	  option += delta;

	  if ((length == 13) || (length == 14))
	    {
	      uint16_t
		l1 = readByte () + 13;
	      if (delta == 14)
		l1 = l1 * 256 + readByte () + 256;

	      length = l1;
	    }
#ifdef DUMP_COAP
	  Serial.print ("option :");
	  Serial.print (option);
	  Serial.print (" length=");
	  Serial.println (length);
#endif
	  switch (option)
	    {
	    case COAP_OPTION_URI_PATH:
#ifdef DUMP_COAP
	      Serial.print ("Uri-Path ");
#endif
	      URIPath += '/';
	      for (int k = 0; k < length; k++)
		{
		  URIPath += (char) readByte ();
		  if (URIPath.length () > URI_LENGTH)
		    {
		      return ERROR_CODE (5, 00);
		    }
		}
#ifdef DUMP_COAP
	      Serial.println (URIPath);
#endif
	      break;

	    default:		// flush option
#ifdef DUMP_COAP
	      Serial.print ("Unknown Option :");
	      Serial.println (option);
#endif
	      for (int k = 0; k < length; k++)
		{
		  b = readByte ();
		}
	      break;
	     
	    }
#ifdef DUMP_COAP
	  Serial.println ();
#endif
	}			// b != 0xFF

    }				// while for options
  if (b == 0xFF) // store data (if exists) in input buffer
    {
      while (lora.dataAvailable())
	input += (char) readByte();
    }

  // filter wrong type 

  if (type == 0x03)
    {				// RST disable observe ?
      Serial.println ("RST MID =");
      Serial.println (mid);
    }


  if (type == 0x02)
    return 0xFF;		// should never receive an ACK

  if ((type != 0x00) && (type != 0x01))
    {				// not CON or NON
      status = ERROR_CODE (4, 00);
      goto proc_error;
    }

  // process method

  switch (code)
    {				// method
    case COAP_METHOD_GET:
      URIres = resList->find(URIPath, CR_READ);

      if (!URIres)
	{
	  status = ERROR_CODE (4, 04);
	  goto proc_error;
	}


        setHeader (((type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON),
		   ERROR_CODE (2, 05), 
		   ((type == COAP_TYPE_CON) ? mid : s_mid++), 
		   &token);	//ACK same MID, NON new
	addOption (COAP_OPTION_CONTENT, format);
	addOption (COAP_END_OPTION);
	addValue (URIres, 0); // change 0 BY FORMAT
	endMessage ();	
	break; // end of METHOD_GET

    case COAP_METHOD_POST:
    case COAP_METHOD_PUT:
      URIres = resList->find (URIPath, CR_WRITE);

      if (!URIres)
	{
	  status = ERROR_CODE (4, 04);
	  goto proc_error;
	}

      status = (*URIres->function_put) (URIres, format, code,  input);
      goto proc_error;
 

    }

 not_coap_err:
  Serial.println("Skipping...");
  while (lora.dataAvailable())  {
  readByte(); 
  }
  Serial.println();

  return status;

 
 proc_error:
#ifdef DUMP_COAP
  Serial.print ("Send Status = ");
  Serial.println (status, HEX);
#endif
  if (status)
    {
      setHeader ( ((type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON),
		 status,
		  ((type == COAP_TYPE_CON) ? mid : s_mid++),
		  &token);
      endMessage ();

    }
  return status;
}




int CoAPServer::incoming()
{
  if (lora.dataAvailable()) {
    processRequest();
    }
}

byte CoAPServer::readByte()
{
  byte b;

   digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    b = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);

#ifdef DEEP_DEBUG
    Serial.print ("<"); Serial.print(b, HEX); Serial.print(">"); 
#endif
    return b;
}


uint32_t CoAPServer::getValue (uint8_t len)
{
  int
    Value = 0;

  for (int k = 0; k < len; k++)
    {
      Value <<= 8;
      Value += readByte ();
    }
  return Value;
}

void
CoAPServer::setHeader (uint8_t type, uint8_t code, uint16_t mid,
		       CoAPToken * token)
{
  byte b[4];

  currentOption = 0;

  idxOB = 0; 

  outgoingBuf[idxOB] = 0x40 | ((type << 4) & 0x30);	// Version = 01
  outgoingBuf[idxOB++] |= (token->tkl & 0x0F);
  outgoingBuf[idxOB++] = code;
  outgoingBuf[idxOB++] = (mid >> 8);
  outgoingBuf[idxOB++] = (mid & 0x00FF);

  for (int k=0; k < token->tkl; k++) outgoingBuf[idxOB++] = token->token[k];
}



void
CoAPServer::addOption (uint8_t T, uint8_t L, byte * V)
{
  byte b;

  if (T == 0xFF)
    {
      b = 0xFF;

      outgoingBuf[idxOB++] = b;

      return;
    }

  if (T < currentOption)
    return;			// only increase

  uint8_t delta = T - currentOption;

  currentOption = T;

  if (delta < 0x0F)
    {
      b = (delta << 4);
    }
  else
    Serial.println ("not implemented yet");

  if (L < 0x0F)
    {
      b |= L;
    }
  else
    Serial.println ("not implemented yet");

  outgoingBuf[idxOB++] = b;
  for (int k = 0; k < L; k++) outgoingBuf[idxOB++] = V[k];
}

void
CoAPServer::addOption (uint8_t T, uint32_t V)
{
  byte b;
  uint8_t L;
  bool opt = true; 

  if (T == 0xFF)
    {
      b = 0xFF;
      outgoingBuf[idxOB++] = b;

      return;
    }

  if (T < currentOption)
    return;			// only increase

  uint8_t delta = T - currentOption;

  currentOption = T;

  byte O[4];
  uint32_t mask = 0xFF000000;
  L = 4;

  for (int k = 0; k < 4; k++)
    {
      O[k] = ((V & mask) >> (3 - k) * 8);
      mask >>= 8;
      if (!O[k] & opt) 
	L--;
      else opt = false;
    }

  if (!L)
    L = 1;			// send a least 1 byte

  if (delta < 0x0F)
    {
      b = (delta << 4);
    }
  else
    Serial.println ("not implemented yet");

  if (L < 0x0F)
    {
      b |= L;
    }
  else
    Serial.println ("not implemented yet");

  outgoingBuf[idxOB++] = b;
  for (int k = 4 - L; k < 4; k++)
    outgoingBuf[idxOB++] = O[k];
}

void
CoAPServer::addValue (CoAPResource* res, uint8_t format)
{
  String result = (*res->function_get) (res, format);

  for (int k =0; k < result.length(); k++) outgoingBuf[idxOB++] = result[k];
}

void
CoAPServer::endMessage ()
{
#ifdef DEEP_DEBUG
  Serial.println ("Sending...");
  for (int i=0; i < idxOB; i++) {
    Serial.print (outgoingBuf[i], HEX);
    Serial.print (" ");
  }
#endif
  Serial.println();

  lora.write (outgoingBuf, idxOB); 
}

//================

CoAPResource* 
CoAPResource::add (String newName, t_answer_get fg, t_answer_put fp, uint8_t type)
{
  CoAPResource *NE = this;


  while (NE->next != NULL)
    {
      if (NE->name == newName)
	{
	  return NE;
	}
      NE = NE->next;
    }
  NE->next = new CoAPResource (newName, fg, fp, type);
  return NE->next;
}

#ifdef DUMP_COAP
void
CoAPResource::list ()
{
  Serial.println ("list ");

  CoAPResource *NE = this;
  while (NE != NULL)
    {
      Serial.println (NE->name);
      NE = NE->next;
    }
}
#endif

CoAPResource *
CoAPResource::find (String targetName, uint8_t type)
{
  CoAPResource *current = this;

  while (current)
    {
      if ((current->name == targetName) && (current->type & type))
	return current;

      current = current->next;
    }

  return NULL;
}

//==

void
CoAPToken::setToken (uint8_t l, byte * p)
{
  tkl = l;

  for (int k = 0; k < l; k++)
    token[k] = p[k];
}

void
CoAPToken::writeToken (byte * p)
{
  for (int k = 0; k < tkl; k++)
    p[k] = token[k];
}

bool CoAPToken::compareToken (uint8_t l, byte * p)
{
  if (tkl != l)
    return false;

  for (int k = 0; k < l; k++)
    if (token[k] != p[k])
      return false;

  return true;
}

void
CoAPToken::copy (CoAPToken * source)
{
  tkl = source->tkl;

  for (int k = 0; k < tkl; k++)
    token[k] = source->token[k];
}


#ifdef DUMP_COAP
void
CoAPToken::printToken ()
{
  for (int k = 0; k < tkl; k++)
    Serial.print (token[k], HEX);
  Serial.println ();
}
#endif


//========
