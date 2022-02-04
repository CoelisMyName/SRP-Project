#include <cstring>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "WaveRender.h"

TAG(WaveRender)

WaveRender::WaveRender() {

}

WaveRender::~WaveRender() {

}

void WaveRender::onAttach() {

}

void WaveRender::onStart(int64_t timestamp) {

}

void WaveRender::onStop(int64_t timestamp) {

}

void WaveRender::onReceive(int64_t timestamp, int16_t *data, int32_t length) {

}

void WaveRender::onDetach() {

}

void WaveRender::onCreate(int32_t width, int32_t height) {

}

void WaveRender::onDraw() {
    //TODO 重复调用的函数，将获取到的幅值绘制到视窗
}

void WaveRender::onChange(int32_t width, int32_t height) {
    AAsset *asset;
    off_t size;
    int rd;
    asset = AAssetManager_open(g_assets, "wave.vert", AASSET_MODE_STREAMING);
    size = AAsset_getLength(asset);
    char *vert = new char[size + 1];
    memset(vert, 0, size + 1);
    rd = AAsset_read(asset, vert, size);
    AAsset_close(asset);
    asset = AAssetManager_open(g_assets, "wave.vert", AASSET_MODE_STREAMING);
    size = AAsset_getLength(asset);
    char *frag = new char[size + 1];
    memset(frag, 0, size + 1);
    rd = AAsset_read(asset, frag, size);
    AAsset_close(asset);
}

void WaveRender::onDestroy() {

}