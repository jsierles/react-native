// Copyright 2004-present Facebook. All Rights Reserved.

#include "Value.h"

#include <jni/fbjni.h>
#include <fb/log.h>

#include "JSCHelpers.h"

namespace facebook {
namespace react {

Value::Value(JSContextRef context, JSValueRef value) :
  m_context(context),
  m_value(value)
{
}

Value::Value(Value&& other) :
  m_context(other.m_context),
  m_value(other.m_value)
{
  other.m_value = nullptr;
}

JSContextRef Value::context() const {
  return m_context;
}

std::string Value::toJSONString(unsigned indent) const {
  JSValueRef exn;
  auto stringToAdopt = JSValueCreateJSONString(m_context, m_value, indent, &exn);
  if (stringToAdopt == nullptr) {
    std::string exceptionText = Value(m_context, exn).toString().str();
    throwJSExecutionException("Exception creating JSON string: %s", exceptionText.c_str());
  }
  return String::adopt(stringToAdopt).str();
}

/* static */
Value Value::fromJSON(JSContextRef ctx, const String& json) {
  auto result = JSValueMakeFromJSONString(ctx, json);
  if (!result) {
    throwJSExecutionException("Failed to create String from JSON");
  }
  return Value(ctx, result);
}

Object Value::asObject() {
  JSValueRef exn;
  JSObjectRef jsObj = JSValueToObject(context(), m_value, &exn);
  if (!jsObj) {
    std::string exceptionText = Value(m_context, exn).toString().str();
    throwJSExecutionException("Failed to convert to object: %s", exceptionText.c_str());
  }
  Object ret = Object(context(), jsObj);
  m_value = nullptr;
  return std::move(ret);
}

Value Object::callAsFunction(int nArgs, JSValueRef args[]) {
  JSValueRef exn;
  JSValueRef result = JSObjectCallAsFunction(m_context, m_obj, NULL, nArgs, args, &exn);
  if (!result) {
    std::string exceptionText = Value(m_context, exn).toString().str();
    throwJSExecutionException("Exception calling JS function: %s", exceptionText.c_str());
  }
  return Value(m_context, result);
}

Value Object::getProperty(String propName) const {
  JSValueRef exn;
  JSValueRef property = JSObjectGetProperty(m_context, m_obj, propName, &exn);
  if (!property) {
    std::string exceptionText = Value(m_context, exn).toString().str();
    throwJSExecutionException("Failed to get property: %s", exceptionText.c_str());
  }
  return Value(m_context, property);
}

Value Object::getProperty(const char *propName) const {
  return getProperty(String(propName));
}

} }
