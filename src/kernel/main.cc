#include "asmfun.h"
#include "fonts.h"
#include "graphics.h"
#include "int.h"
#include "queue.h"
#include "mouse.h"
#include "keyboard.h"
#include "memory.h"
#include "layer.h"

Layer* layer_back;
Layer* layer_mouse;
Layer* layer_log;

static char mouse_info[20];
static char mem_info[20];

void init_system();
void init_layer();

void create_window(int x, int y, int w, int h, const char title[]) {
    Layer* layer = LayerManager::alloc((uint8_t*)MemoryManager::alloc(w*h), w, h, -1);
    LayerManager::slide(layer, x, y);
    LayerManager::updown(layer, 100);
    static char close_icon[14][17] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };

    fill_box(0, 0,      w, 1, LightGrey, layer);
    fill_box(1, 1,      w-1, 2, White, layer);
    fill_box(0, 0,      1, h, LightGrey, layer);
    fill_box(1, 1,      2, h-1, White, layer);
    fill_box(w-2, 1,    w-1, h-1, LightGrey, layer);
    fill_box(w-1, 0,    w, h, Black, layer);
    fill_box(2, 2,      w-3, h-3, LightGrey, layer);
    fill_box(3, 3,      w-5, 20, Blue, layer);
    fill_box(1, h-2,    w-1, h-1, DarkGrey, layer);
    fill_box(0, h-1,    w, h, Black, layer);

    draw_string(title, 5, 5, White, layer);
    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 16; j++) {
            char c;
            switch (close_icon[i][j]) {
                case '@': c = Black; break;
                case '$': c = DarkGrey; break;
                case 'Q': c = LightGrey; break;
                default: c = White; break;
            }
            layer->buff[(5+i)*layer->width + (layer->width-21+j)] = c;
        }
    }

    draw_string("Welcome to", 10, 30, White, layer);
    fill_box(10, 30, strlen("Welcome to")*8 + 1, 17, LightGrey, layer);
    draw_string("PianOS", 10, 46, Black, layer);

    LayerManager::refresh(x, y, w, h);
}

int main(void) {
    init_system();
    init_layer();
    create_window(50, 50, 120, 120, "Window");
    while(true) {
        cli();
        if (!keyboard_buff.empty()) {
            uint8_t data = keyboard_buff.front();
            keyboard_buff.pop();
            sti();
        }
        if (!mouse_buff.empty()) {
            int mouse_date_size = 3;
            static int8_t data[3];  // 有符号接收
            static int read_status;
            //舍弃刚开始0xfa
            if (is_mouse_init == false && mouse_buff.front() == 0xfa) {
                is_mouse_init = true;
                mouse_buff.pop();  
                read_status = 0;
                sti();
            } else if (is_mouse_init) { // 初始化完成
                data[read_status++] = mouse_buff.front();
                mouse_buff.pop();
                sti();
                if (read_status == mouse_date_size) { // 读完2字节
                    Mouse tm = mouse;
                    mouse_decode(data);
                    LayerManager::slide(layer_mouse, mouse.x, mouse.y);  
                    sprintf(mouse_info, "(%d, %d)", mouse.x, mouse.y);
                    draw_string(mouse_info, 0, 0, 10, layer_log);
                    read_status = 0;
                } 
            }
        }
        sti(); 
    }
    return 0;
}


void init_system() {
    MemoryManager::init();
    int idt = get_idt();
    set_idt_seg((IDT_Descriptor*)idt + 0x21, (int)asm_response_keyboard, 8, 0x008e);
    set_idt_seg((IDT_Descriptor*)idt + 0x2c, (int)asm_response_mouse, 8, 0x008e);

    out_byte(0x21, 0b11111001); /* 开放PIC1和键盘中断(11111001) */
	out_byte(0xa1, 0b11101111); /* 开放鼠标中断(11101111) */

    init_keyboard();
    enable_mouse();
    is_mouse_init = false;

    MemoryManager::free((void*)0, MemoryManager::size);
    MemoryManager::alloc(1474560);  // 1.44MB 后作为堆
}

void init_layer() {
//==============桌面层==============
    layer_back = LayerManager::alloc(
        (uint8_t*)MemoryManager::alloc(win_w*win_h), 
        ::scrn_w, ::scrn_h, -1);  // 设置图层缓冲区
        
    LayerManager::slide(layer_back, 0, 0);                      // 图层在（0，0）位置
    LayerManager::updown(layer_back, 0);
    draw_desktop(layer_back);

//==============log层==============
    layer_log = LayerManager::alloc(
        (uint8_t*)MemoryManager::alloc(win_w*win_h), 
        ::scrn_w, ::scrn_h, 99);
    //  图层置为透明
    for (int i = 0; i < ::scrn_h * ::scrn_w; i++) {
        layer_log->buff[i] = 99;
    }

    LayerManager::slide(layer_log, 0, 0);
    LayerManager::updown(layer_log, 1);
    sprintf(mem_info, "Memory:%dKB Free:%dKB", MemoryManager::size / 1024 + 1024, MemoryManager::total() / 1024 + 1024);
    sprintf(mouse_info, "(%d, %d)", mouse.x, mouse.y);
    draw_string(mouse_info, 0, 0, 10, layer_log);
    draw_string(mem_info, 0, 16, 10, layer_log);
     
    
//==============鼠标层==============
    layer_mouse = LayerManager::alloc(
        (uint8_t*)MemoryManager::alloc(curosr_size*curosr_size), 
        curosr_size, curosr_size, 99);

    LayerManager::slide(layer_mouse, mouse.x, mouse.y);
    LayerManager::updown(layer_mouse, 2);
    draw_cursor(0, 0, layer_mouse);
}
