//
//
//
#include <pico/stdlib.h>

#include <array>
#include <atomic>
#include <iostream>

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
void luaInit() {
  luaState = luaL_newstate();
  luaL_openlibs(luaState);

  lua_register(luaState, "DrawString", lDrawString);
  lua_register(luaState, "FillRect", lFillRect);
  lua_register(luaState, "DrawRect", lDrawRect);
  lua_register(luaState, "DrawLine", lDrawLine);
  lua_register(luaState, "LCDClear", lClear);
  lua_register(luaState, "GetColor", lGetColor);
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
                         u16_t response_uri_len) {}
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
