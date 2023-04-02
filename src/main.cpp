//
//
//
#include <pico/stdlib.h>

#include <array>
#include <atomic>
#include <iostream>
#include <memory>

#include "lwip/apps/httpd.h"
#include "lwipopts.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

extern "C" {
#include "DEV_Config.h"
#include "LCD_Bmp.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "LCD_Touch.h"
#include "ff.h"

extern LCD_DIS sLCD_DIS;
}
#include <lua.hpp>

#include "draw.h"
#include "font.h"

using namespace Katakori;

namespace {

Font font1;
std::string recvBuffer = "empty";
lua_State *luaState = nullptr;
enum ExecState : int8_t { Free, Request, Execute };
std::atomic_int8_t execState = Free;

//
const char *echo_cgi_handler(int index, int num_params, char *pc_param[],
                             char *pc_value[]) {
  // for (int i = 0; i < num_params; i++) {
  //   if (strcmp(pc_param[i], "text") == 0) {
  //     recvBuffer = pc_value[i];
  //   }
  // }
  return "/cgi.html";
}

std::array<const tCGI, 1> echo_cgi_table = {
    {"/echo.cgi", echo_cgi_handler},
};

//
//
//

//
int lClear(lua_State *l) {
  auto col = luaL_checkinteger(l, -1);
  LCD_Clear(col);
  return 0;
}

//
int lGetColor(lua_State *l) {
  auto colr = luaL_checkinteger(l, -3);
  auto colg = luaL_checkinteger(l, -2);
  auto colb = luaL_checkinteger(l, -1);
  lua_pushinteger(l, Color(colr, colg, colb));
  return 1;
}

//
int lFillRect(lua_State *l) {
  auto xl = luaL_checkinteger(l, -5);
  auto yt = luaL_checkinteger(l, -4);
  auto xr = luaL_checkinteger(l, -3);
  auto yb = luaL_checkinteger(l, -2);
  auto col = luaL_checkinteger(l, -1);
  FillRect(xl, yt, xr, yb, col);
  return 0;
}

//
int lDrawRect(lua_State *l) {
  auto xl = luaL_checkinteger(l, -5);
  auto yt = luaL_checkinteger(l, -4);
  auto xr = luaL_checkinteger(l, -3);
  auto yb = luaL_checkinteger(l, -2);
  auto col = luaL_checkinteger(l, -1);
  DrawRect(xl, yt, xr, yb, col);
  return 0;
}

//
int lDrawLine(lua_State *l) {
  auto xl = luaL_checkinteger(l, -5);
  auto yt = luaL_checkinteger(l, -4);
  auto xr = luaL_checkinteger(l, -3);
  auto yb = luaL_checkinteger(l, -2);
  auto col = luaL_checkinteger(l, -1);
  DrawLine(xl, yt, xr, yb, col);
  return 0;
}

//
int lDrawString(lua_State *l) {
  size_t len{};
  auto xpos = luaL_checkinteger(l, -4);
  auto ypos = luaL_checkinteger(l, -3);
  auto col = luaL_checkinteger(l, -2);
  auto msg = luaL_checklstring(l, -1, &len);
  if (len > 0) {
    DrawString(xpos, ypos, col, msg, &font1);
  }

  return 0;
}

//
int lSetGramScanWay(lua_State *l) {
  auto dir = static_cast<LCD_SCAN_DIR>(luaL_checkinteger(l, -1));
  LCD_SetGramScanWay(dir);
  return 0;
}

std::shared_ptr<FATFS> sdFatFs;
//
int lFMount(lua_State *l) {
  FRESULT res = FR_OK;
  if (!sdFatFs) {
    DEV_Digital_Write(SD_CS_PIN, 1);
    // DEV_Digital_Write(LCD_CS_PIN, 1);
    // DEV_Digital_Write(TP_CS_PIN, 1);

    sdFatFs = std::make_shared<FATFS>();
    res = f_mount(sdFatFs.get(), "/", 1);
    if (res != FR_OK) {
      sdFatFs.reset();
    }
  }
  lua_pushinteger(l, res);
  return 1;
}

//
int lFUnmount(lua_State *l) {
  FRESULT res = FR_OK;
  if (sdFatFs) {
    res = f_mount(nullptr, "/", 1);
    if (res == FR_OK) {
      sdFatFs.reset();
    }
  }
  lua_pushinteger(l, res);
  return 1;
}

//
int lFMkdir(lua_State *l) {
  auto dirname = luaL_checkstring(l, -1);
  auto res = f_mkdir(dirname);
  lua_pushinteger(l, res);
  return 1;
}

//
int lFCreate(lua_State *l) {
  auto fname = luaL_checkstring(l, -1);
  auto file = static_cast<FIL *>(lua_newuserdata(l, sizeof(FIL)));
  auto res = f_open(file, fname, FA_CREATE_ALWAYS | FA_WRITE);
  lua_pushinteger(l, res);
  return 2;
}

//
int lFOpen(lua_State *l) {
  auto fname = luaL_checkstring(l, -1);
  auto file = static_cast<FIL *>(lua_newuserdata(l, sizeof(FIL)));
  auto res = f_open(file, fname, FA_READ | FA_WRITE);
  lua_pushinteger(l, res);
  return 2;
}

//
int lFClose(lua_State *l) {
  auto file = static_cast<FIL *>(lua_touserdata(l, -1));
  auto res = f_close(file);
  lua_pushinteger(l, res);
  return 1;
}

//
int lFWrite(lua_State *l) {
  auto file = static_cast<FIL *>(lua_touserdata(l, -2));
  auto data = luaL_checkstring(l, -1);
  auto len = strlen(data);
  UINT num = 0;
  auto res = f_write(file, data, len, &num);
  lua_pushinteger(l, num);
  lua_pushinteger(l, res);
  return 2;
}

//
int lFRead(lua_State *l) {
  auto file = static_cast<FIL *>(lua_touserdata(l, -1));
  luaL_Buffer buff;
  auto *readBuff = luaL_buffinitsize(l, &buff, file->fsize);
  UINT num = 0;
  auto res = f_read(file, readBuff, file->fsize, &num);
  luaL_pushresultsize(&buff, num);
  lua_pushinteger(l, res);
  return 2;
}

//
int lFSeek(lua_State *l) {
  auto file = static_cast<FIL *>(lua_touserdata(l, -2));
  auto pos = luaL_checkinteger(l, -1);
  auto res = f_lseek(file, pos);
  lua_pushinteger(l, res);
  return 1;
}
//
void luaInit() {
  luaState = luaL_newstate();
  luaL_openlibs(luaState);

  lua_register(luaState, "DrawString", lDrawString);
  lua_register(luaState, "FillRect", lFillRect);
  lua_register(luaState, "DrawRect", lDrawRect);
  lua_register(luaState, "DrawLine", lDrawLine);
  lua_register(luaState, "LCDClear", lClear);
  lua_register(luaState, "GetColor", lGetColor);
  lua_register(luaState, "SetGramScanWay", lSetGramScanWay);
  lua_register(luaState, "FMount", lFMount);
  lua_register(luaState, "FUnmount", lFUnmount);
  lua_register(luaState, "FMkdir", lFMkdir);
  lua_register(luaState, "FCreate", lFCreate);
  lua_register(luaState, "FOpen", lFOpen);
  lua_register(luaState, "FClose", lFClose);
  lua_register(luaState, "FWrite", lFWrite);
  lua_register(luaState, "FRead", lFRead);
  lua_register(luaState, "FSeek", lFSeek);
}

}  // namespace

extern "C" {

err_t httpd_post_begin(void *connection, const char *uri,
                       const char *http_request, u16_t http_request_len,
                       int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
  std::string sURI = uri;
  if (sURI == "/script") {
    // strncpy(response_uri, "/cgi.html", 10);
    // response_uri_len = 10;
    return ERR_OK;
  }
  return ERR_VAL;
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
  auto ret = ERR_VAL;
  if (p != nullptr) {
    if (execState.load() == ExecState::Free) {
      recvBuffer.resize(p->len);
      memcpy(recvBuffer.data(), p->payload, p->len);
      execState = ExecState::Request;
      ret = ERR_OK;
    }
    pbuf_free(p);
  }
  return ret;
}

void httpd_post_finished(void *connection, char *response_uri,
                         u16_t response_uri_len) {
  strncpy(response_uri, "/cgi.html", 10);
  response_uri_len = 10;
}
}

//
//
//
int main() {
  System_Init();

  if (cyw43_arch_init()) {
    std::cerr << "failed: network initialized." << std::endl;
    return 1;
  }

  cyw43_arch_enable_sta_mode();

  std::cout << "Connecting to Wi-Fi..." << std::endl;

  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    std::cerr << "failed: network connecting" << std::endl;
    return 1;
  }

  httpd_init();

  http_set_cgi_handlers(echo_cgi_table.data(), echo_cgi_table.size());

  auto lcd_scan_dir = LCD_SCAN_DIR::L2R_U2D;
  LCD_Init(lcd_scan_dir, 40);
  TP_Init(lcd_scan_dir);
  TP_GetAdFac();

  LCD_Clear(Gray);
  if (!font1.AssignMappedMemory((const void *)(0x10000000 + 0x100000))) {
    std::cerr << "failed: font loading" << std::endl;
    return 1;
  }

  {
    extern cyw43_t cyw43_state;
    auto ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;

    std::array<char, 64> msg{};
    snprintf(msg.data(), msg.size(), "IP: %d.%d.%d.%d", ip_addr & 0xff,
             (ip_addr >> 8) & 0xff, (ip_addr >> 16) & 0xff, ip_addr >> 24);
    DrawString(20, 10, White, msg.data(), &font1);
  }

  luaInit();

  int cnt = 0;
  for (;;) {
    TP_Update();

    if (execState.load() == ExecState::Request) {
      execState = ExecState::Execute;
      luaL_dostring(luaState, recvBuffer.c_str());
      execState = ExecState::Free;

      std::array<char, 16> cntBuff;
      snprintf(cntBuff.data(), cntBuff.size(), "E: %d", ++cnt);
      FillRect(10, 290, 100, 310, Gray);
      DrawString(10, 290, White, cntBuff.data(), &font1);
    }

    sleep_ms(16);
  }

  return 0;
}
//
