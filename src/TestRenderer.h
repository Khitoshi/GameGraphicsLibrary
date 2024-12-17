#pragma once
#include "Test.h"

class TestRenderer : public Test
{
public:
    bool Initialize()   override;
    void Run()          override;
    void Shutdown()     override;
};

