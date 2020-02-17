//
// Created by 赵帆 on 2020-02-14.
//

#include "IObserver.h"

void IObserver::addObserver(IObserver *observer) {
    if (!observer) return;
    mux.lock();
    this->observers.push_back(observer);
    mux.unlock();
}

void IObserver::notify(XData &data) {
    mux.lock();
    int size = observers.size();
    for (int i = 0; i < size; i++) {
        observers[i]->update(data);
    }
    mux.unlock();
}
