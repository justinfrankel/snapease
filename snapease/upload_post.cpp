/*

  Example PHP:


<?php
$output_root = dirname(__FILE__);
if ($output_root == "") die("error on server\n");

if ($_SERVER['PHP_AUTH_USER'] != 'test' || $_SERVER['PHP_AUTH_PW'] != 'password')
{
   header('WWW-Authenticate: Basic realm="snapease_upload"');
   header('HTTP/1.0 401 Unauthorized');
   die('error bad password\n');
}

if (!$_FILES || !($file = $_FILES["snapease_file"])) die("error no file\n");

$tgt = str_replace("\\","/",$_REQUEST['snapease_target']);
for (;;) // remove any questionable .. from filenames!
{
  $x = str_replace("../","/",$tgt);
  $x = str_replace("/..","/",$x);
  $x = str_replace("//","/",$x);
  if ($x==$tgt) break;
  $tgt=$x;
}

if ($tgt == "") die("error invalid target specified\n");

// optional safety checks
if (strcasecmp(substr($tgt,-4),".jpg") && strcasecmp(substr($tgt,-4),".png")) die("error bad filename!\n");
//if (@file_exists("$output_root/$tgt")) die("error output file already exists!\n");

$a = explode("/",$tgt);
$tmp = $output_root;
for ($x=0;$x<count($a)-1;$x++)
{
  $tmp .= "/" . $a[$x];
  @mkdir($tmp); // todo mode?
}

if (!@move_uploaded_file($file["tmp_name"], "$output_root/$tgt")) die("error copying file to '$output_root/$tgt'\n");

echo "ok\n";

?>




*/


#include "main.h"
#include "../WDL/jnetlib/jnetlib.h"
#include "../WDL/fileread.h"

#include "uploader.h"

#define POST_DIV_STRING "zzzASFIJAHFASJFHASLKFHZI8VZJKZ__________AZZ8530597329562798067FZJXXXX"

class PostUploader : public IFileUploader
{
  public:
    PostUploader()
    {
      static bool init;
      if (!init)
      {
        init=true;
        JNL::open_socketlib();
      }
      m_con=0;
      m_fr=0;
      m_errorstate=0;
      m_linestate=0;
    }

    virtual ~PostUploader() 
    { 
      delete m_fr;
      delete m_con;
    }

    virtual bool SendFile(const char *srcfullfn, const char *destfn); // true if success
    virtual int Run(char *statusBuf, int statusBufLen); // >0 completed, <0 error (statusBuf will be error text)

    WDL_String m_extrapost_content; // stuff to send at end

    JNL_IConnection *m_con;
    WDL_FileRead *m_fr;
    int m_file_size_sending;
    int m_file_send_pos;

    int m_errorstate;

    int m_linestate;

    WDL_String m_errstr;
};

static void AddTextField(WDL_String *s, const char *name, const char *value)
{
  s->AppendFormatted(4096,"--" POST_DIV_STRING "\r\n"
                          "Content-Disposition: form-data; name=\"%s\"\r\n"
                          "\r\n"
                          "%s\r\n",
                              name,
                              value);
}


bool PostUploader::SendFile(const char *srcfullfn, const char *destfn) // true if success
{
  WDL_String m_host("http://www.1014.org/test/upload.php");
  WDL_String m_login("test");
  WDL_String m_pass("password");
  WDL_String m_leadpath("blah");

  m_errorstate=0;
  m_linestate=0;

  delete m_fr;
  delete m_con;
  m_fr=0;
  m_con=0;

  m_fr = new WDL_FileRead(srcfullfn);
  if (!m_fr->IsOpen()) 
  {
    m_errorstate=-1;
    return false;
  }

  const char *hsrc = m_host.Get();
  if (!strnicmp(hsrc,"http://",7)) hsrc+=7;
  WDL_String hb(hsrc);
  int port=80;
  char zb[32]={0,};
  char *req = zb, *parms=zb;
  char *p=hb.Get();
  while (*p && *p != ':' && *p != '/' && *p != '?') p++;
  if (*p == ':')
  {
    *p++=0;
    port = atoi(p);
    if (!port) port=80;
  }
  while (*p && *p != '/' && *p != '?') p++;
  if (*p == '/')
  {
    *p++=0;
    req = p;
  } 
  
  m_con = new JNL_Connection(JNL_CONNECTION_AUTODNS,65536,65536);
  m_con->connect(hb.Get(), port);

  m_extrapost_content.Set("\r\n");
  {
    char tgt[1024];
    lstrcpyn(tgt,m_leadpath.Get(),300);
    strcat(tgt,"/");
    lstrcpyn(tgt+strlen(tgt),destfn,300);
    char *in = tgt, *out=tgt;
    while (*in)
    {
      switch (*in)
      {
        case '/':
        case '\\': 
          if (out==tgt || out[-1] != '/') *out++='/'; 
        break;
        
        default:
          if (out!=in) *out=*in;
          out++;
        break;
      }
      in++;
    }
    *out=0;
    AddTextField(&m_extrapost_content,"snapease_target",tgt);
  }
  m_extrapost_content.Append("--" POST_DIV_STRING "--\r\n");


  const char *initialcontent = "--" POST_DIV_STRING "\r\n"
                        "Content-Disposition: form-data; name=\"snapease_file\"; filename=\"snapease_random_filename.dat\"\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Content-transfer-encoding: binary\r\n"
                        "\r\n";

  

  WDL_INT64 fs=m_fr->GetSize();
  m_file_size_sending = fs<1 ? 0 : fs > 1024*1024*1024 ? 1024*1024*1024 : (int) fs;


  char lpbuf[1024];
  lpbuf[0]=0;
  if (m_login.Get()[0]||m_pass.Get()[0])
  {
    WDL_String tmp;
    tmp.Set(m_login.Get());
    tmp.Append(":");
    tmp.Append(m_pass.Get());
    if (strlen(tmp.Get()) > 256) tmp.Get()[255]=0;

    strcpy(lpbuf,"Authorization: Basic ");
    JNL_HTTPGet::do_encode_mimestr(tmp.Get(),lpbuf+strlen(lpbuf));
    strcat(lpbuf,"\r\n");
  }

  WDL_String tmp;
  tmp.SetFormatted(4096,"POST /%s HTTP/1.1\r\n"
                        "Connection: close\r\n"
                        "Host: %s\r\n"
                        "User-Agent: Cockos SnapEase (Mozilla)\r\n"
                        "MIME-Version: 1.0\r\n"
                        "Content-type: multipart/form-data; boundary=" POST_DIV_STRING "\r\n"
                        "Content-length: %d\r\n"
                        "%s\r\n",
                        req,
                        hb.Get(),
                        m_file_size_sending + strlen(initialcontent) + strlen(m_extrapost_content.Get()),
                        lpbuf
                        );

  m_con->send_string(tmp.Get());

  m_con->send_string(initialcontent);

  // sending file!
  m_file_send_pos=0;

  return true;
}

int PostUploader::Run(char *statusBuf, int statusBufLen) // >0 completed, <0 error (statusBuf will be error text)
{
  if (m_errorstate)
  {
    switch (m_errorstate)
    {
      case -5:
        lstrcpyn(statusBuf,m_errstr.Get(),statusBufLen);
      break;
      case -4:
        lstrcpyn(statusBuf,"Generic post: Error reading from source file",statusBufLen);
      break;
      case -3:
        lstrcpyn(statusBuf,"Generic post: Connection error",statusBufLen);
      break;
      case -2:
        lstrcpyn(statusBuf,"Generic post: Connection closed before upload completed",statusBufLen);
      break;
      case -1:
        lstrcpyn(statusBuf,"Generic post: Error opening file on disk",statusBufLen);
      break;
      default:
        lstrcpyn(statusBuf,"Generic post: Unknown error",statusBufLen);
      break;
    }
    return -1;
  }
  m_con->run();

  if (!m_linestate)
  {
    if (m_con->recv_lines_available()>0)
    {
      char buf[4096];
      buf[0]=0;
      m_con->recv_line(buf,sizeof(buf));
      if (!strstr(buf,"200"))
      {
        m_errstr.SetFormatted(1024,"Generic Post: got HTTP response: '%.300s'",buf);
        m_errorstate=-5;
        return 0;
      }
      m_linestate=1;
    }
  }
  if (m_linestate==1)
  {
    while (m_con->recv_lines_available()>0 && m_linestate==1)
    {
      char buf[4096];
      buf[0]=0;
      m_con->recv_line(buf,sizeof(buf));
      if (!buf[0]) m_linestate=2;
    }
  }
  if (m_linestate==2)
  {
    if (m_con->recv_lines_available()>0)
    {
      char buf[4096];
      buf[0]=0;
      m_con->recv_line(buf,sizeof(buf));
      if (stricmp(buf,"ok"))
      {
        m_errstr.SetFormatted(1024,"Generic Post: got script reply: '%.300s'",buf);
        m_errorstate=-5;
        return 0;
      }
      else
      {
        m_linestate=3;
      }
    }
  }
  if (m_linestate==3)
  {
    if (!m_con->send_bytes_in_queue()) 
    {
      m_con->close(0);
      m_linestate=4;
    }
  }
  int state = m_con->get_state();
  // todo verify response
  switch (state)
  {
    case JNL_Connection::STATE_CLOSED:
      if (m_file_send_pos >= m_file_size_sending)
      {
        lstrcpyn(statusBuf,"Upload completed",statusBufLen);
        return 1;
      }
      m_errorstate=-2;
    return 0;
    case JNL_Connection::STATE_NOCONNECTION:
    case JNL_Connection::STATE_ERROR:
      m_errorstate=-3;
    return 0;
    case JNL_Connection::STATE_RESOLVING:
      lstrcpyn(statusBuf,"Resolving host",statusBufLen);
    return 0;
    case JNL_Connection::STATE_CONNECTING:
      lstrcpyn(statusBuf,"Connecting to host",statusBufLen);
    return 0;
  }



  int maxsend = m_file_size_sending-m_file_send_pos;
  if (maxsend > m_con->send_bytes_available())
    maxsend = m_con->send_bytes_available();

  if (maxsend>0)
  {
    char buf[4096];
    if (maxsend >sizeof(buf)) maxsend=sizeof(buf);
    if (m_fr->Read(buf,maxsend) != maxsend)
    {
      m_errorstate=-4;
      return 0;
    }
    m_file_send_pos+=maxsend;
    m_con->send_bytes(buf,maxsend);
  }
  if (m_file_send_pos >= m_file_size_sending)
  {
    if (m_extrapost_content.Get()[0] && m_con->send_bytes_available() >= strlen(m_extrapost_content.Get()))
    {
      m_con->send_string(m_extrapost_content.Get());
      m_extrapost_content.Set("");
    }
  }
  {
    char tmp[512];
    sprintf(tmp,"Sending %d/%d",m_file_send_pos,m_file_size_sending);
    lstrcpyn(statusBuf,tmp,statusBufLen);
  }

  return 0;
}


IFileUploader *CreateGenericPostUploader()
{
  return new PostUploader;
}