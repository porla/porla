#pragma once

namespace porla::Actions
{
    struct ActionCallback
    {
        virtual void Invoke(bool success) = 0;
    };
}
