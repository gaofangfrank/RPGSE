#include "SaveElements.h"

#include <QJsonObject>

//=============================================================================
// TODO's
Actors::Actors(QJsonValueRef val) : Base::Base(val) {}
Party::Party(QJsonValueRef val) : Base::Base(val) {}
Variables::Variables(QJsonValueRef val) : Base::Base(val) {}
Switches::Switches(QJsonValueRef val) : Base::Base(val) {}

//=============================================================================

SaveElements::SaveElements(QJsonObject &save)
    : actors(save["actors"]), party(save["party"]),
      variables(save["variables"]), switches(save["Switches"]) {}

bool SaveElements::reload(QJsonObject &save) {
  this->actors = save["actors"];
  this->party = save["party"];
  this->variables = save["variables"];
  this->switches = save["switches"];

  return actors.isValid() && party.isValid() && variables.isValid() &&
         switches.isValid();
}
