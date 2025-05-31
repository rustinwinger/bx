#ifndef _MODES_MELEE20BUTTON_HPP
#define _MODES_MELEE20BUTTON_HPP

#include "core/ControllerMode.hpp"
#include "core/socd.hpp"
#include "core/state.hpp"
#include "MeleeLimits.hpp"

typedef struct {
    bool crouch_walk_os = false;
    bool teleport_coords = false;
} Melee20ButtonOptions;

class Melee20Button : public ControllerMode {
  public:
    Melee20Button(socd::SocdType socd_type, Melee20ButtonOptions options = {});
    bool isMelee();

  protected:
    void UpdateDigitalOutputs(InputState &inputs, OutputState &outputs);
    void UpdateAnalogOutputs(InputState &inputs, OutputState &outputs);

  private:
    Melee20ButtonOptions _options;
    bool _horizontal_socd;

    void HandleSocd(InputState &inputs);
};

#endif
