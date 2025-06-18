#ifndef SAVELEMENTS_H
#define SAVELEMENTS_H

#include <QJsonValueRef>
class QJsonObject;

template <typename Derived> class SaveElement {
public:
  SaveElement() = delete;
  SaveElement(QJsonValueRef val) : jsonObj(val), valid(!val.isNull()) {}

  bool isValid() const { return valid; }

protected:
  using Base = SaveElement<Derived>;
  QJsonValueRef jsonObj;
  bool valid;
};
class Actors : public SaveElement<Actors> {
public:
  Actors(QJsonValueRef val, const QJsonArray actorDefs);

private:
  struct ActorParams {
    QString name;
    int32_t hp;
    int32_t mp;
    int32_t level;
    std::vector<std::pair<int, int32_t>> exp;
    int32_t classId;
  };
  std::unordered_map<int, ActorParams> params;
};

class Party : public SaveElement<Party> {
public:
  Party(QJsonValueRef val);

private:
  int32_t gold;
  std::unordered_map<size_t, int32_t> itemCount;
  std::unordered_map<size_t, int32_t> weaponCount;
  std::unordered_map<size_t, int32_t> armorCount;
};

class Variables : public SaveElement<Variables> {
public:
  Variables(QJsonValueRef val);
};

class Switches : public SaveElement<Switches> {
public:
  Switches(QJsonValueRef val);
};

class SaveElements {
public:
  SaveElements() = delete;
  SaveElements(QJsonObject save, const QJsonArray actorList);
  bool reload(QJsonObject save, const QJsonArray actorList);
  bool isValid();

private:
  Actors actors;
  Party party;
  Variables variables;
  Switches switches;
};
#endif
