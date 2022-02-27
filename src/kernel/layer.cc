#include "layer.h"

void Layer::set_buff(uint8_t* buff, int width, int height, int alpha) {
    // MemoryManager::free(buff, width * height);
    // this->buff = (uint8_t*)MemoryManager::alloc(width * height);
    this->buff = buff;
    this->width = width;
    this->height = height;
    this->alpha = alpha;
}

int32_t LayerManager::top = 0;
Layer* LayerManager::sheets[Max_Layer];
Layer LayerManager::sheets0[Max_Layer];

LayerManager::LayerManager() {
}

Layer* LayerManager::alloc() {
    Layer* layer = nullptr;
    for (int i = 0; i < Max_Layer; i++) {
        if (sheets0[i].flags == 0) {
            layer = sheets0 + i;
            layer->flags = 1;
            layer->z_index = -1;
            break;
        }
    }
    return layer;
}

void LayerManager::updown(Layer* layer, int z_index) {
    layer->z_index = z_index;
    sheets[z_index] = layer;
    
    int old_z_index = layer->z_index;
    z_index = min(z_index, top + 1);
    z_index = max(z_index, -1);

    layer->z_index = z_index;

    if (old_z_index > z_index) { // 比以前低
        if (z_index >= 0) {
            for (int i = old_z_index; i > z_index; i--) {
                sheets[i] = sheets[i-1];
                sheets[i]->z_index = i;
            }
            sheets[z_index] = layer;
        }
        else {
            if (top > old_z_index) {
                for (int i = old_z_index; i < top; i++) {
                    sheets[i] = sheets[i+1];
                    sheets[i]->z_index = i;
                }
            }
            top--;
        }
    }        
    else if (old_z_index < z_index) { // 比以前低
        if (old_z_index >= 0) {
            for (int i = old_z_index; i < z_index; i++) {
                sheets[i] = sheets[i+1];
                sheets[i]->z_index = i;
            }
            sheets[z_index] = layer;
        }
        else {
            for (int i = top; i >= z_index; i--) {
                sheets[i+1] = sheets[i];
                sheets[i+1]->z_index = i + 1;
            }
            sheets[z_index] = layer;
            top++;
        }
    }
    refresh(layer->x, layer->y, layer->width, layer->height);
}

// void LayerManager::refresh() {
//     for (int z = 0; z < Max_Layer; z++) {
//         Layer* layer = sheets[z];
//         if (layer->flags == 0) {
//             continue;
//         }
//         // 绘制当前层
//         for (int y = 0; y < layer->height; y++) {
//             int ry = layer->y + y;
//             for (int x = 0; x < layer->width; x++) {
//                 int rx = layer->x + x;
//                 ::vram[ry * scrn_w + rx] = layer->buff[y * layer->width +x];
//             }
//         }
//     }
// }

void LayerManager::refresh(int x, int y, int w, int h) {

    x = max(x, 0);
    y = max(y, 0);
    w = min(scrn_w - x, w);
    h = min(scrn_h - y, h);
    // if (x + w > scrn_w) {
    //     w = scrn_w - x;
    // }

    for (int z = 0; z < Max_Layer; z++) {
        Layer* layer = sheets[z];
        if (layer->flags == 0) {
            continue;
        }

        int bx0 = x - layer->x;
        int by0 = y - layer->y;
        int bx1 = x + w - layer->x;
        int by1 = y + h - layer->y;

        bx0 = max(bx0, 0);
        by0 = max(by0, 0);

        bx1 = min(layer->width, bx1);
        by1 = min(layer->height, by1);

        for (int i = by0; i < by1; i++) {
            int ry = i + layer->y;
            for (int j = bx0; j < bx1; j++) {
                int rx = j + layer->x;
                ::vram[ry*scrn_w + rx] =
                    layer->buff[i*layer->width + j];
            }
        }
    }
}

void LayerManager::slide(Layer* layer, int x, int y) {
    int old_x = layer->x;
    int old_y = layer->y;
    layer->x = x;
    layer->y = y;
    if (layer->z_index >= 0) {
        refresh(old_x, old_y, layer->width, layer->height);
        refresh(x, y, layer->width, layer->height);
        // refresh();
    }
}

void LayerManager::free(Layer* layer) {
    if (layer->z_index >= 0) {
        updown(layer, -1);
    }
    layer->flags = 0;
}