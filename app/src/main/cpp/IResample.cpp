//
// Created by 赵帆 on 2020-02-16.
//

#include "IResample.h"

void IResample::update(XData &data) {
    XData resampleData = this->resample(data);
    if (resampleData.size > 0) {
        this->notify(resampleData);
    }

}