#include "SaveElements.h"

#include <QDebug>

static json::Value *traverseJsonObj(json::Object &obj,
                                    std::initializer_list<const char *> keys) {
  json::Value *val = nullptr;
  json::Object *cur = &obj;
  for (const char *key : keys) {
    if (!cur)
      return nullptr;
    val = cur->get(key);
    if (!val)
      return nullptr;
    cur = val->getAsObject();
  }
  return val;
}

static void parseCount(json::Object *obj,
                       std::unordered_map<size_t, json::Value *> &count) {
  for (auto &[key, val] : *obj) {
    // Test for numeric value
    char firstChar = key.str()[0];
    if (firstChar < '0' || firstChar > '9')
      continue;
    size_t id = std::stoi(key.str());
    count[id] = &val;
  }
}

//=============================================================================
// Actors
Actors::Actors(json::Object &save, json::Array &actorDefs) {
  json::Value *maybeVal = traverseJsonObj(save, {"actors", "_data", "@a"});
  if (!maybeVal) {
    qDebug() << "Invalid actor structure in save";
    valid = false;
    return;
  }
  auto *actorList = maybeVal->getAsArray();

  if (!actorList || actorList->size() != actorDefs.size()) {
    qDebug() << "Actors in save mismatch with actor data: " << actorList->size()
             << " vs. " << actorDefs.size();
    valid = false;
    return;
  }

  for (auto tup : llvm::enumerate(*actorList, actorDefs)) {
    int idx = tup.index();
    json::Value &saveVal = std::get<0>(tup.value());
    const json::Value &defVal = std::get<1>(tup.value());

    switch (defVal.kind()) {
    case json::Value::Null:
      continue;
    case json::Value::Object:
      break;
    default:
      qDebug() << "Expecting actor data to be either null or object";
      valid = false;
      return;
    }

    auto *save = saveVal.getAsObject();
    std::vector<std::pair<int, json::Value *>> exp;

    json::Value *name = save->get("_name");
    json::Value *hp = save->get("_hp");
    json::Value *mp = save->get("_mp");
    json::Value *level = save->get("_level");
    json::Value *classId = save->get("_classId");

    // exp is stored in another object, I think the key is the class id that the
    // exp is corresponding to
    json::Object *expObj = save->getObject("_exp");
    if (!expObj) {
      qDebug() << "Expecting object for actor exp";
      valid = false;
      return;
    }
    for (auto &p : *expObj) {
      auto key = p.first;
      // Check if this is a number
      char firstChar = key.str()[0];
      if (firstChar < '0' || firstChar > '9')
        continue;
      int id = std::stoi(key.str());
      exp.emplace_back(id, &p.getSecond());
    }

    params[idx] = {name, hp, mp, level, exp, classId};
  }
}

//=============================================================================
// Party
Party::Party(json::Object &save) {
  auto *partyObj = save.getObject("party");
  if (!partyObj) {
    valid = false;
    qDebug() << "Cannot find party object in save";
    return;
  }

  this->gold = partyObj->get("_gold");

  // Parse items
  auto *items = partyObj->getObject("_items");
  if (!items)
    valid = false;
  else
    parseCount(items, itemCount);

  // Parse weapons
  auto *weapons = partyObj->getObject("_weapons");
  if (!weapons)
    valid = false;
  else
    parseCount(weapons, weaponCount);

  // Parse armors
  auto *armors = partyObj->getObject("_armors");
  if (!armors)
    valid = false;
  else
    parseCount(armors, armorCount);
}

//=============================================================================
// Variables
Variables::Variables(json::Object &save, json::Object &system) {
  json::Value *var = traverseJsonObj(save, {"variables", "_data", "@a"});
  if (!var) {
    valid = false;
    qDebug() << "Unexpected variable layout";
    return;
  }

  // errs() << *var << "\n";
  auto *varArr = var->getAsArray();
  if (!varArr) {
    valid = false;
    qDebug() << "Unexpected variable layout";
    return;
  }

  // errs() << *system.get("variables") << "\n\n";
  auto *names = system.getArray("variables");
  if (!names) {
    valid = false;
    qDebug() << "Unexpected system variable layout";
    return;
  }

  for (auto [val, name] : llvm::zip_first(*varArr, *names)) {
    if (name.kind() != json::Value::String) {
      valid = false;
      qDebug() << "Expecting system variable names to be strings";
      return;
    }
    QString key = name.getAsString().value().str().c_str();
    this->vars[key] = &val;
  }
}

//=============================================================================
Switches::Switches(json::Object &save, json::Object &system) {
  json::Value *switchVal = traverseJsonObj(save, {"switches", "_data", "@a"});
  if (!switchVal) {
    valid = false;
    qDebug() << "Unexpected switch layout";
    return;
  }

  auto *switches = switchVal->getAsArray();
  if (!switches) {
    valid = false;
    qDebug() << "Unexpected switch layout";
    return;
  }

  auto *names = system.getArray("switches");
  if (!names) {
    valid = false;
    qDebug() << "Unexpected system switch layout";
    return;
  }

  for (auto [name, val] : llvm::zip_equal(*names, *switches)) {
    if (name.kind() != json::Value::String) {
      valid = false;
      qDebug() << "Expecting system variable names to be strings";
      return;
    }
    QString key = name.getAsString().value().str().c_str();
    this->switches[key] = &val;
  }
}

//=============================================================================
// SaveElements
SaveElements::SaveElements(json::Object &save, json::Array &actorList,
                           json::Object &system)
    : actors(save, actorList), party(save), variables(save, system),
      switches(save, system) {}

bool SaveElements::isValid() {
  return actors.isValid() && party.isValid() && variables.isValid() &&
         switches.isValid();
}

bool SaveElements::reload(json::Object &save, json::Array &actorList,
                          json::Object &system) {
  this->actors = Actors(save, actorList);
  this->party = save;
  this->variables = Variables(save, system);
  this->switches = Switches(save, system);

  return isValid();
}
