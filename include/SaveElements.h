#ifndef SAVELEMENTS_H
#define SAVELEMENTS_H

#include <QJsonValueRef>
class QJsonObject;

template <typename Derived> class SaveElement {
public:
  SaveElement() = delete;
  SaveElement(QJsonValueRef val) : jsonObj(val) {}

  bool isValid() const { return valid; }

protected:
  using Base = SaveElement<Derived>;
  QJsonValueRef jsonObj;
  bool valid;
};

class Actors : public SaveElement<Actors> {
public:
  Actors(QJsonValueRef val);
};

class Party : public SaveElement<Party> {
public:
  Party(QJsonValueRef val);
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
  SaveElements(QJsonObject &save);
  bool reload(QJsonObject &save);

private:
  Actors actors;
  Party party;
  Variables variables;
  Switches switches;
};
#endif
