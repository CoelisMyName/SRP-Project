#include "log.h"
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

}

void WaveRender::onDestroy() {

}