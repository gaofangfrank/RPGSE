#ifndef SAVELEMENTS_H
#define SAVELEMENTS_H

#include "llvm/Support/JSON.h"

#include <QMap>

using namespace llvm;
template <typename Derived> class SaveElement {
public:
  SaveElement() : valid(true) {}

  bool isValid() const { return valid; }

protected:
  using Base = SaveElement<Derived>;
  bool valid;
};

//=============================================================================
// Actors
class Actors : public SaveElement<Actors> {
public:
  Actors(json::Object &save, json::Array &actorDefs);

private:
  struct ActorParams {
    json::Value *name;
    json::Value *hp;
    json::Value *mp;
    json::Value *level;
    std::vector<std::pair<int, json::Value *>> exp;
    json::Value *classId;
  };
  std::map<int, ActorParams> params;
};

//=============================================================================
// Party
class Party : public SaveElement<Party> {
public:
  Party(json::Object &save);

private:
  json::Value *gold;
  std::unordered_map<size_t, json::Value *> itemCount;
  std::unordered_map<size_t, json::Value *> weaponCount;
  std::unordered_map<size_t, json::Value *> armorCount;
};

//=============================================================================
// Variables
class Variables : public SaveElement<Variables> {
public:
  Variables(json::Object &save, json::Object &system);

private:
  QMap<QString, json::Value *> vars;
};

//=============================================================================
// Switches
class Switches : public SaveElement<Switches> {
public:
  Switches(json::Object &save, json::Object &system);

private:
  QMap<QString, json::Value *> switches;
};

//=============================================================================
// SaveElements
class SaveElements {
public:
  SaveElements() = delete;
  SaveElements(json::Object &save, json::Array &actorList,
               json::Object &system);
  bool reload(json::Object &save, json::Array &actorList, json::Object &system);
  bool isValid();

private:
  Actors actors;
  Party party;
  Variables variables;
  Switches switches;
};
#endif
