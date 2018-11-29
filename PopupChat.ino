// PopupChat - a one-screen system to share things locally with your friends via WIFI
// based on: Captive Portal by: M. Ray Burnette 20150831
// homo est bulae

#include <U8g2lib.h>

//U8g2 Contructor
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);
// Alternative board version. Uncomment if above doesn't work.
// U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 4, /* clock=*/ 14, /* data=*/ 2);

u8g2_uint_t offset;     // current offset for the scrolling text
u8g2_uint_t width;      // pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined
char text[50] = "JULIO "; // scroll this text from right to left

String msg;
//=================================================

#include <ESP8266WiFi.h>
#include "./DNSServer.h"                  // Patched lib
#include <ESP8266WebServer.h>

// config
#define CHATNAME "Badge do Julio"
#define BLURB "Envie texto para o meu ESP8266"
#define COMPLAINTSTO "github.com/tlack/popup-chat"
#define INDEXTITLE "Badge chat"
#define INDEXBANNER ".."
#define POSTEDTITLE "Message posted!"
#define POSTEDBANNER ".."
const String FAQ = ""
"<br/>"
"<br/>"
"<br/>";

// boring
#define VER "..."
const byte HTTP_CODE = 200; // nyi? 511; // rfc6585
const byte DNS_PORT = 53;  // Capture DNS requests on port 53
const byte TICK_TIMER = 1000;
const byte ACTIVITY_DURATION = 60 * TICK_TIMER; // how many seconds should the LED stay on after last visit?
const byte ACTIVITY_LED = 2;
const byte ACTIVITY_REVERSE = 1; // turn off when active, not on.. needed for me
IPAddress APIP(10, 10, 10, 1);    // Private network for server
// state:
String allMsgs="<i>*system restarted*</i>";
unsigned long bootTime=0, lastActivity=0, lastTick=0, tickCtr=0; // timers
DNSServer dnsServer; ESP8266WebServer webServer(80); // standard api servers
void em(String s){ Serial.print(s); } 
void emit(String s){ Serial.println(s); } // debugging
String input(String argName) {
  String a=webServer.arg(argName);
  a.replace("<","&lt;");a.replace(">","&gt;");
  a.substring(0,200); return a; }
String quote() { 
  const byte nquotes=3;
  String quotes[nquotes]={
    "@jcldf", 
    "linkedin.com/in/juliodellaflora",
    "facebook.com/juliodellaflora",
  };
  return quotes[millis() / 1000 / 60 / 60 % nquotes];
}
String footer() { return 
  "</div><div class=q><label>Me procura no: </label>"+quote()+"</div>"
  "<div class=com>Parcialmente Kibado de: " COMPLAINTSTO "</div>"
  "<div class=by>" VER "</div></body></html>"; }
String header(String t) {
  String a = String(CHATNAME);
  String CSS = "article { background: #f2f2f2; padding: 1em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0; }"
    "input { border-radius: 0; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #8b0000; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.2em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
    "<head><title>"+a+" :: "+t+"</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>"+a+"</b> "+BLURB+"</nav><div><h1>"+t+"</h1></div><div>";
  emit("header - "+t);
  emit(h);
  return h; }
String faq() {
  return header("frequently asked questions") + FAQ + footer();
}
String index() {
  return header(INDEXTITLE) + "<div>" + INDEXBANNER + "</div><div><label>Ultimas mensagens:</label><ol>"+allMsgs+
    "</ol></div><div><form action=/post method=post><label>Escreva uma palavra:</label><br/>"+
    "<i>DICA:</i> Escreva aqui para aparecer na badge </i><br/>"+
    "<textarea name=m></textarea><br/><input type=submit value=send></form>" + footer();
}
String posted() {
  msg=input("m"); allMsgs="<li>"+msg+"</li>"+allMsgs;
  emit("posted: "+msg); 
  return header(POSTEDTITLE) + POSTEDBANNER + "<article>"+msg+"</article><a href=/>Back to index</a>" + footer();
 
 
  
}
void setup() {
  Serial.begin(115200); 
  emit("setup"); 
  bootTime = lastActivity = millis();
  pinMode(ACTIVITY_LED, OUTPUT); led(1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(CHATNAME);
  dnsServer.start(DNS_PORT, "*", APIP);
  webServer.on("/post",[]() { webServer.send(HTTP_CODE, "text/html", posted()); });
  webServer.on("/faq",[]() { webServer.send(HTTP_CODE, "text/html", faq()); });
  webServer.onNotFound([]() { lastActivity=millis(); webServer.send(HTTP_CODE, "text/html", index()); });
  webServer.begin();

//===============================
 u8g2.begin();

  u8g2.setFont(u8g2_font_logisoso32_tf); // set the target font to calculate the pixel width
  width = u8g2.getUTF8Width(text);    // calculate the pixel width of the text

  u8g2.setFontMode(0);    // enable transparent mode, which is faster


//===============================



  
}
void led(byte p){
  byte on=p^ACTIVITY_REVERSE; emit("led"+String(on));
  digitalWrite(ACTIVITY_LED, on ? HIGH : LOW);
}
void tick() {
  String tickCs=String(tickCtr++); // emit("tick #"+tickCs+" @"+String(millis()));
  if ((millis() - lastActivity) < ACTIVITY_DURATION) {
    em("+"); led(1);
  } else {
    em("-"); lastActivity = 0; led(0);
  }
}
void loop() { 
  if ((millis()-lastTick)>TICK_TIMER) {lastTick=millis(); tick();} 
  dnsServer.processNextRequest(); webServer.handleClient(); 

//================================

u8g2_uint_t x;

  u8g2.firstPage();
  do {
  msg.toCharArray(text, 50);
    // draw the scrolling text at current offset
    x = offset;
    u8g2.setFont(u8g2_font_logisoso32_tf);   // set the target font
    do {                // repeated drawing of the scrolling text...
      u8g2.drawUTF8(x, 32, text);     // draw the scolling text
      x += width;           // add the pixel width of the scrolling text
    } while ( x < u8g2.getDisplayWidth() );   // draw again until the complete display is filled

    u8g2.setFont(u8g2_font_logisoso32_tf);   // draw the current pixel width
    u8g2.setCursor(0, 64);
    u8g2.print(width);          // this value must be lesser than 128 unless U8G2_16BIT is set

  } while ( u8g2.nextPage() );

  offset -= 1;            // scroll by one pixel
  if ( (u8g2_uint_t)offset < (u8g2_uint_t) - width )
    offset = 0;             // start over again
//================================
  
  
  }
