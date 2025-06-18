#include "SaveElements.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

static void parseCount(const QJsonObject json,
                       std::unordered_map<size_t, int32_t> &counter) {
  for (auto it = json.begin(); it != json.end(); ++it) {
    const QString key = it.key();
    bool ok;
    size_t keyval = key.toULong(&ok);
    if (!ok)
      continue;
    counter[keyval] = it.value().toInt();
  }
}

template <typename JSONT,
          std::enable_if_t<std::is_same<JSONT, QJsonObject>::value ||
                               std::is_same<JSONT, QJsonArray>::value ||
                               std::is_same<JSONT, QString>::value ||
                               std::is_same<JSONT, bool>::value ||
                               std::is_same<JSONT, int>::value ||
                               std::is_same<JSONT, double>::value,
                           bool> = true>
static bool traverseJsonObj(const QJsonObject obj,
                            std::initializer_list<const char *> keys,
                            JSONT &val) {
  QJsonValue result = obj;
  for (const char *key : keys) {
    if (!result.isObject())
      return false;
    QJsonObject nextObj = result.toObject();
    result = std::move(nextObj[key]);
  }
  if constexpr (std::is_same<JSONT, QJsonObject>()) {
    if (!result.isObject())
      return false;
    val = std::move(result.toObject());
  }
  if constexpr (std::is_same<JSONT, QJsonArray>()) {
    if (!result.isArray())
      return false;
    val = std::move(result.toArray());
  }
  if constexpr (std::is_same<JSONT, QString>()) {
    if (!result.isString())
      return false;
    val = result.toString();
  }
  if constexpr (std::is_same<JSONT, bool>()) {
    if (!result.isBool())
      return false;
    val = result.toBool();
  }
  if constexpr (std::is_same<JSONT, int>()) {
    if (!result.isDouble())
      return false;
    val = result.toInt();
  }
  if constexpr (std::is_same<JSONT, double>()) {
    if (!result.isDouble())
      return false;
    val = result.toDouble();
  }
  return true;
}

//=============================================================================
Actors::Actors(QJsonValueRef val, const QJsonArray actorDefs)
    : Base::Base(val) {
  if (!valid || !val.isObject()) {
    qDebug() << "Invalid Actors";
    return;
  }

  const QJsonObject obj = val.toObject();
  QJsonArray actorList;
  if (!traverseJsonObj(obj, {"_data", "@a"}, actorList)) {
    qDebug() << "Invalid actor structure in save";
    valid = false;
    return;
  }

  if (actorList.size() != actorDefs.size()) {
    qDebug() << "Actors in save mismatch with actor data: " << actorList.size()
             << " vs. " << actorDefs.size();
    valid = false;
    return;
  }

  int idx = 0;
  for (auto defIt = actorDefs.begin(), saveIt = actorList.constBegin();
       defIt != actorDefs.end(); ++defIt, ++saveIt, ++idx) {
    if (defIt->isNull())
      continue;
    if (!defIt->isObject() || !saveIt->isObject()) {
      qDebug() << "Expecting actor data to be either null or object";
      valid = false;
      return;
    }
    const auto def = defIt->toObject();
    QString name;
    if (!traverseJsonObj(def, {"name"}, name) || name.size() == 0)
      continue;

    const auto save = saveIt->toObject();
    int hp, mp, level, classId;
    std::vector<std::pair<int, int32_t>> exp;

    traverseJsonObj(save, {"_name"}, name);
    traverseJsonObj(save, {"_hp"}, hp);
    traverseJsonObj(save, {"_mp"}, mp);
    traverseJsonObj(save, {"_level"}, level);
    QJsonObject expObj;
    if (traverseJsonObj(save, {"_exp"}, expObj)) {
      for (auto it = expObj.begin(); it != expObj.end(); ++it) {
        bool ok;
        int maybeClass = it.key().toInt(&ok);
        if (!ok)
          continue;
        exp.emplace_back(maybeClass, it->toInt());
      }
    }
    traverseJsonObj(save, {"_classId"}, classId);

    params[idx] = {name, hp, mp, level, exp, classId};
  }
}

//=============================================================================
// Party
Party::Party(QJsonValueRef val) : Base::Base(val) {
  if (!valid) {
    qDebug() << "Invalid Party";
    return;
  }
  const QJsonObject obj = val.toObject();
  this->gold = obj["_gold"].toInt();

  // Parse items
  const QJsonObject items = obj["_items"].toObject();
  parseCount(items, itemCount);

  // Parse weapons
  const QJsonObject weapons = obj["_weapons"].toObject();
  parseCount(weapons, weaponCount);

  // Parse armors
  const QJsonObject armors = obj["_armors"].toObject();
  parseCount(armors, armorCount);
}

//=============================================================================
Variables::Variables(QJsonValueRef val) : Base::Base(val) {
  if (!valid) {
    qDebug() << "Invalid Variables";
    return;
  }
}

//=============================================================================
Switches::Switches(QJsonValueRef val) : Base::Base(val) {
  if (!valid) {
    qDebug() << "Invalid Switches";
    return;
  }
}

//=============================================================================
// SaveElements
SaveElements::SaveElements(QJsonObject save, const QJsonArray actorList)
    : actors(save["actors"], actorList), party(save["party"]),
      variables(save["variables"]), switches(save["switches"]) {}

bool SaveElements::isValid() {
  return actors.isValid() && party.isValid() && variables.isValid() &&
         switches.isValid();
}

bool SaveElements::reload(QJsonObject save, const QJsonArray actorList) {
  this->actors = Actors(save["actors"], actorList);
  this->party = save["party"];
  this->variables = save["variables"];
  this->switches = save["switches"];

  return isValid();
}
