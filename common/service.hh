#pragma once

namespace mylife {

  class service {
  public:
    virtual ~service() = default;

    virtual void setup() {
    }

    virtual void loop() {
    }
  };
}
