// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD//// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at////     http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
//https://github.com/v12345vtm/ESP32-CAM-IPCAM
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "fb_gfx.h"//houden
#include "fd_forward.h" //houden

static const char PROGMEM INDEX2_HTML[] = R"rawliteral(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>v5 almost SD-card  v12345vtm ESP32-CAM-IPCAM</title>
<style>
body{font-family:Arial,Helvetica,sans-serif;background:#f2ff3f;color:#000000;font-size:16px}h2{font-size:18px}section.main{display:flex}#menu,section.main{flex-direction:column}#menu{display:none;flex-wrap:nowrap;min-width:340px;background:#666666;padding:8px;border-radius:4px;margin-top:-10px;margin-right:10px}#content{display:flex;flex-wrap:wrap;align-items:stretch}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}figure img{display:block;width:100%;height:auto;border-radius:4px;margin-top:8px}@media (min-width: 800px) and (orientation:landscape){#content{display:flex;flex-wrap:nowrap;align-items:stretch}figure img{display:block;max-width:100%;max-height:calc(100vh - 40px);width:auto;height:auto}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}}section#buttons{display:flex;flex-wrap:nowrap;justify-content:space-between}#nav-toggle{cursor:pointer;display:block}#nav-toggle-cb{outline:0;opacity:0;width:0;height:0}#nav-toggle-cb:checked+#menu{display:flex}.input-group{display:flex;flex-wrap:nowrap;line-height:22px;margin:5px 0}.input-group>label{display:inline-block;padding-right:10px;min-width:47%}.input-group input,.input-group select{flex-grow:1}.range-max,.range-min{display:inline-block;padding:0 5px}button{display:block;margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#ff3034;border-radius:5px;font-size:16px;outline:0}button:hover{background:#ff494d}button:active{background:#f21c21}button.disabled{cursor:default;background:#a0a0a0}input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#363636;cursor:pointer;margin:0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}input[type=range]:focus::-webkit-slider-runnable-track{background:#EFEFEF}input[type=range]::-moz-range-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-moz-range-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer}input[type=range]::-ms-track{width:100%;height:2px;cursor:pointer;background:0 0;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-fill-upper{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;height:2px}input[type=range]:focus::-ms-fill-lower{background:#EFEFEF}input[type=range]:focus::-ms-fill-upper{background:#363636}.switch{display:block;position:relative;line-height:22px;font-size:16px;height:22px}.switch input{outline:0;opacity:0;width:0;height:0}.slider{width:50px;height:22px;border-radius:22px;cursor:pointer;background-color:grey}.slider,.slider:before{display:inline-block;transition:.4s}.slider:before{position:relative;content:"";border-radius:50%;height:16px;width:16px;left:4px;top:3px;background-color:#fff}input:checked+.slider{background-color:#ff3034}input:checked+.slider:before{-webkit-transform:translateX(26px);transform:translateX(26px)}select{border:1px solid #363636;font-size:14px;height:22px;outline:0;border-radius:5px}.image-container{position:relative;min-width:160px}.close{position:absolute;right:5px;top:5px;background:#ff3034;width:16px;height:16px;border-radius:100px;color:#fff;text-align:center;line-height:18px;cursor:pointer}.hidden{display:none}
</style>
</head>
<body>
<section class="main">
<div id="logo">    
</div>
<div id="content">
<div id="sidebar">
<input type="checkbox" id="nav-toggle-cb" checked="checked">
<nav id="menu">
<section id="buttons">
<button id="get-still">Get Still or surf to /capture</button>
<button id="toggle-stream">Start Stream @port9601 </button>
<p><a href="/capture">foto</a></p>
<p><a href=":9601/stream">stream</a></p>
</section>
</nav>
</div>
<figure>
<div id="stream-container" class="image-container hidden">
<div class="close" id="close-stream">×</div>
<img id="stream" src="">
</div>
</figure>
</div>
</section>
<script>
document.addEventListener('DOMContentLoaded',function(){function b(B){let C;switch(B.type){case'checkbox':C=B.checked?1:0;break;case'range':case'select-one':C=B.value;break;case'button':case'submit':C='1';break;default:return;}const D=`${c}/control?var=${B.id}&val=${C}`;fetch(D).then(E=>{console.log(`request to ${D} finished, status: ${E.status}`)})}var c=document.location.origin;const e=B=>{B.classList.add('hidden')},f=B=>{B.classList.remove('hidden')},g=B=>{B.classList.add('disabled'),B.disabled=!0},h=B=>{B.classList.remove('disabled'),B.disabled=!1},i=(B,C,D)=>{D=!(null!=D)||D;let E;'checkbox'===B.type?(E=B.checked,C=!!C,B.checked=C):(E=B.value,B.value=C),D&&E!==C?b(B):!D&&('aec'===B.id?C?e(v):f(v):'agc'===B.id?C?(f(t),e(s)):(e(t),f(s)):'awb_gain'===B.id?C?f(x):e(x):'face_recognize'===B.id&&(C?h(n):g(n)))};document.querySelectorAll('.close').forEach(B=>{B.onclick=()=>{e(B.parentNode)}}),fetch(`${c}/status`).then(function(B){return B.json()}).then(function(B){document.querySelectorAll('.default-action').forEach(C=>{i(C,B[C.id],!1)})});const j=document.getElementById('stream'),k=document.getElementById('stream-container'),l=document.getElementById('get-still'),m=document.getElementById('toggle-stream'),n=document.getElementById('face_enroll'),o=document.getElementById('close-stream'),p=()=>{window.stop(),m.innerHTML='Start Stream'},q=()=>{j.src=`${c+':9601'}/stream`,f(k),m.innerHTML='Stop Stream'};l.onclick=()=>{p(),j.src=`${c}/capture?_cb=${Date.now()}`,f(k)},o.onclick=()=>{p(),e(k)},m.onclick=()=>{const B='Stop Stream'===m.innerHTML;B?p():q()},n.onclick=()=>{b(n)},document.querySelectorAll('.default-action').forEach(B=>{B.onchange=()=>b(B)});const r=document.getElementById('agc'),s=document.getElementById('agc_gain-group'),t=document.getElementById('gainceiling-group');r.onchange=()=>{b(r),r.checked?(f(t),e(s)):(e(t),f(s))};const u=document.getElementById('aec'),v=document.getElementById('aec_value-group');u.onchange=()=>{b(u),u.checked?e(v):f(v)};const w=document.getElementById('awb_gain'),x=document.getElementById('wb_mode-group');w.onchange=()=>{b(w),w.checked?f(x):e(x)};const y=document.getElementById('face_detect'),z=document.getElementById('face_recognize'),A=document.getElementById('framesize');A.onchange=()=>{b(A),5<A.value&&(i(y,!1),i(z,!1))},y.onchange=()=>{return 5<A.value?(alert('Please select CIF or lower resolution before enabling this feature!'),void i(y,!1)):void(b(y),!y.checked&&(g(n),i(z,!1)))},z.onchange=()=>{return 5<A.value?(alert('Please select CIF or lower resolution before enabling this feature!'),void i(z,!1)):void(b(z),z.checked?(h(n),i(y,!0)):g(n))}});
</script>
</body>
</html>


)rawliteral";
///////////



typedef struct {
size_t size; //number of values used for filtering
size_t index; //current value index
size_t count; //value count
int sum;
int * values; //array to be filled with values
} ra_filter_t;

typedef struct {
httpd_req_t *req;
size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
 
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;


 
  // Function for stopping the webserver
//void stop_webserver(httpd_handle_t  server)//stop de foto capture server
void stop_fotowebserver() //stop de foto server vanuit de ino-file
 {
 httpd_handle_t server = camera_httpd; //overwriten parameter omdat we vanuit de ino-file geen parameter kunnen meegeven
      // Ensure handle is non NULL
      if (server != NULL) {
          // Stop the httpd server
          httpd_stop(server);
           Serial.printf("fotoserver gestopt \n");  
      }
 }

void stop_streamwebserver() //stop de foto server vanuit de ino-file
 {
 httpd_handle_t server = stream_httpd; //overwriten parameter omdat we vanuit de ino-file geen parameter kunnen meegeven
      // Ensure handle is non NULL
      if (server != NULL) {
          // Stop the httpd server
          httpd_stop(server);
           Serial.printf("streamserver gestopt \n");  
      }
 }


 

static esp_err_t capture_handler(httpd_req_t *req) {
  Serial.println("v12345vtm okcapture req=");
  Serial.println((long) req); // voorbeeld = 1073560312 constante?
camera_fb_t * fb = NULL;// make room for the image to recieve
esp_err_t res = ESP_OK;
int64_t fr_start = esp_timer_get_time();//get millies timestamp
  Serial.println("esp_timer_get_time()=");
    Serial.println( (long) fr_start); //voorbeeld  = varieert vb: 13085182 of 21219612
fb = esp_camera_fb_get(); //methode in esp_camera.h to get image from cam
if (!fb) {
Serial.println("Camera capture failed");
httpd_resp_send_500(req);
return ESP_FAIL;
}
httpd_resp_set_type(req, "image/jpeg");
httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg"); //antwoord header an capture pagina
size_t fb_len = 0; //metadata
fb_len = fb->len;//metadata used in serialprint only
res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
esp_camera_fb_return(fb);
int64_t fr_end = esp_timer_get_time();
Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));
return res; 
}

static esp_err_t stream_handler(httpd_req_t *req) {
camera_fb_t * fb = NULL;
esp_err_t res = ESP_OK;
size_t _jpg_buf_len = 0;
uint8_t * _jpg_buf = NULL;
char * part_buf[64];
dl_matrix3du_t *image_matrix = NULL;

//int face_id = 0;
int64_t fr_start = 0;
int64_t fr_ready = 0;
static int64_t last_frame = 0;
if (!last_frame) {
last_frame = esp_timer_get_time();
}

res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
if (res != ESP_OK) {
return res;
}

while (true) { //continue streaming until it breaks 
  fb = esp_camera_fb_get();
  if (!fb) {
  Serial.println("Camera capture failed");
  res = ESP_FAIL;
  } 
    else
  {
  fr_start = esp_timer_get_time();
  fr_ready = fr_start; 
//  Serial.println("fbwidth=");
 // Serial.print(fb->width );//1600 pixels
  _jpg_buf_len = fb->len;
  _jpg_buf = fb->buf;   
    }
  
  if (res == ESP_OK) {     
  size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
  res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
  }
  if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
  }
  if (res == ESP_OK) {
       res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
  }
  if (fb) {
  esp_camera_fb_return(fb);
  fb = NULL;
  _jpg_buf = NULL;
  } else if (_jpg_buf) {
  free(_jpg_buf);
  _jpg_buf = NULL;
  }
  if (res != ESP_OK) {
    Serial.printf("break stream \n");  
  break;
  }
  int64_t fr_end = esp_timer_get_time();  
  Serial.printf("MJPG: \n");  
  }
    last_frame = 0;
  return res;
  }
  
  
  static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  Serial.printf("webpage loading \n");
  return httpd_resp_send(req, (const char *)INDEX2_HTML, strlen(INDEX2_HTML));
  }
 
  void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG(); //see esp_http_server.h
 
  httpd_uri_t index_uri = {
  .uri       = "/",
  .method    = HTTP_GET,
  .handler   = index_handler,
  .user_ctx  = NULL
  };
  
  httpd_uri_t capture_uri = {
  .uri       = "/capture", //v12345vtm
  .method    = HTTP_GET,
  .handler   = capture_handler,
  .user_ctx  = NULL
  };
  
  httpd_uri_t stream_uri = {
  .uri       = "/stream",
  .method    = HTTP_GET,
  .handler   = stream_handler,
  .user_ctx  = NULL
  };
  
    Serial.printf("CPP Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
  httpd_register_uri_handler(camera_httpd, &index_uri);
  httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
  
  config.server_port += 1;//in esp_http_server.h 
  config.ctrl_port += 1;//in esp_http_server.h 
  
  config.server_port = 9601; //stream//in esp_http_server.h 
  
  
  Serial.printf("CPP Starting stream server on stream port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
  httpd_register_uri_handler(stream_httpd, &stream_uri);
}

//stop_webserver(camera_httpd); //stop de foto capture server

  
 
  }
