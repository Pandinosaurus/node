// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_SOURCE_TEXT_MODULE_H_
#define V8_OBJECTS_SOURCE_TEXT_MODULE_H_

#include "src/objects/module.h"

// Has to be the last include (doesn't have include guards):
#include "src/objects/object-macros.h"

namespace v8 {
namespace internal {

class UnorderedModuleSet;

// The runtime representation of an ECMAScript Source Text Module Record.
// https://tc39.github.io/ecma262/#sec-source-text-module-records
class SourceTextModule : public Module {
 public:
  NEVER_READ_ONLY_SPACE
  DECL_CAST(SourceTextModule)
  DECL_VERIFIER(SourceTextModule)
  DECL_PRINTER(SourceTextModule)

  // The code representing this module, or an abstraction thereof.
  // This is either a SharedFunctionInfo, a JSFunction, a JSGeneratorObject, or
  // a SourceTextModuleInfo, depending on the state (status) the module is in.
  // See SourceTextModule::SourceTextModuleVerify() for the precise invariant.
  DECL_ACCESSORS(code, Object)

  // Arrays of cells corresponding to regular exports and regular imports.
  // A cell's position in the array is determined by the cell index of the
  // associated module entry (which coincides with the variable index of the
  // associated variable).
  DECL_ACCESSORS(regular_exports, FixedArray)
  DECL_ACCESSORS(regular_imports, FixedArray)

  // The shared function info in case {status} is not kEvaluating, kEvaluated or
  // kErrored.
  SharedFunctionInfo GetSharedFunctionInfo() const;

  // Modules imported or re-exported by this module.
  // Corresponds 1-to-1 to the module specifier strings in
  // SourceTextModuleInfo::module_requests.
  DECL_ACCESSORS(requested_modules, FixedArray)

  // [script]: Script from which the module originates.
  DECL_ACCESSORS(script, Script)

  // The value of import.meta inside of this module.
  // Lazily initialized on first access. It's the hole before first access and
  // a JSObject afterwards.
  DECL_ACCESSORS(import_meta, Object)

  // Get the SourceTextModuleInfo associated with the code.
  inline SourceTextModuleInfo info() const;

  Cell GetCell(int cell_index);
  static Handle<Object> LoadVariable(Isolate* isolate,
                                     Handle<SourceTextModule> module,
                                     int cell_index);
  static void StoreVariable(Handle<SourceTextModule> module, int cell_index,
                            Handle<Object> value);

  static int ImportIndex(int cell_index);
  static int ExportIndex(int cell_index);

  // Get the namespace object for [module_request] of [module].  If it doesn't
  // exist yet, it is created.
  static Handle<JSModuleNamespace> GetModuleNamespace(
      Isolate* isolate, Handle<SourceTextModule> module, int module_request);

  // Layout description.
  DEFINE_FIELD_OFFSET_CONSTANTS(Module::kHeaderSize,
                                TORQUE_GENERATED_SOURCE_TEXT_MODULE_FIELDS)

  using BodyDescriptor =
      SubclassBodyDescriptor<Module::BodyDescriptor,
                             FixedBodyDescriptor<kCodeOffset, kSize, kSize>>;

 private:
  friend class Factory;
  friend class Module;

  // TODO(neis): Don't store those in the module object?
  DECL_INT_ACCESSORS(dfs_index)
  DECL_INT_ACCESSORS(dfs_ancestor_index)

  // Helpers for Instantiate and Evaluate.

  static void CreateExport(Isolate* isolate, Handle<SourceTextModule> module,
                           int cell_index, Handle<FixedArray> names);
  static void CreateIndirectExport(Isolate* isolate,
                                   Handle<SourceTextModule> module,
                                   Handle<String> name,
                                   Handle<SourceTextModuleInfoEntry> entry);

  static V8_WARN_UNUSED_RESULT MaybeHandle<Cell> ResolveExport(
      Isolate* isolate, Handle<SourceTextModule> module,
      Handle<String> module_specifier, Handle<String> export_name,
      MessageLocation loc, bool must_resolve, ResolveSet* resolve_set);
  static V8_WARN_UNUSED_RESULT MaybeHandle<Cell> ResolveImport(
      Isolate* isolate, Handle<SourceTextModule> module, Handle<String> name,
      int module_request, MessageLocation loc, bool must_resolve,
      ResolveSet* resolve_set);

  static V8_WARN_UNUSED_RESULT MaybeHandle<Cell> ResolveExportUsingStarExports(
      Isolate* isolate, Handle<SourceTextModule> module,
      Handle<String> module_specifier, Handle<String> export_name,
      MessageLocation loc, bool must_resolve, ResolveSet* resolve_set);

  static V8_WARN_UNUSED_RESULT bool PrepareInstantiate(
      Isolate* isolate, Handle<SourceTextModule> module,
      v8::Local<v8::Context> context, v8::Module::ResolveCallback callback);
  static V8_WARN_UNUSED_RESULT bool FinishInstantiate(
      Isolate* isolate, Handle<SourceTextModule> module,
      ZoneForwardList<Handle<SourceTextModule>>* stack, unsigned* dfs_index,
      Zone* zone);
  static V8_WARN_UNUSED_RESULT bool RunInitializationCode(
      Isolate* isolate, Handle<SourceTextModule> module);

  static void FetchStarExports(Isolate* isolate,
                               Handle<SourceTextModule> module, Zone* zone,
                               UnorderedModuleSet* visited);

  static V8_WARN_UNUSED_RESULT MaybeHandle<Object> Evaluate(
      Isolate* isolate, Handle<SourceTextModule> module,
      ZoneForwardList<Handle<SourceTextModule>>* stack, unsigned* dfs_index);

  static V8_WARN_UNUSED_RESULT bool MaybeTransitionComponent(
      Isolate* isolate, Handle<SourceTextModule> module,
      ZoneForwardList<Handle<SourceTextModule>>* stack, Status new_status);

  static void Reset(Isolate* isolate, Handle<SourceTextModule> module);

  OBJECT_CONSTRUCTORS(SourceTextModule, Module);
};

// SourceTextModuleInfo is to SourceTextModuleDescriptor what ScopeInfo is to
// Scope.
class SourceTextModuleInfo : public FixedArray {
 public:
  DECL_CAST(SourceTextModuleInfo)

  static Handle<SourceTextModuleInfo> New(Isolate* isolate, Zone* zone,
                                          SourceTextModuleDescriptor* descr);

  inline FixedArray module_requests() const;
  inline FixedArray special_exports() const;
  inline FixedArray regular_exports() const;
  inline FixedArray regular_imports() const;
  inline FixedArray namespace_imports() const;
  inline FixedArray module_request_positions() const;

  // Accessors for [regular_exports].
  int RegularExportCount() const;
  String RegularExportLocalName(int i) const;
  int RegularExportCellIndex(int i) const;
  FixedArray RegularExportExportNames(int i) const;

#ifdef DEBUG
  inline bool Equals(SourceTextModuleInfo other) const;
#endif

 private:
  friend class Factory;
  friend class SourceTextModuleDescriptor;
  enum {
    kModuleRequestsIndex,
    kSpecialExportsIndex,
    kRegularExportsIndex,
    kNamespaceImportsIndex,
    kRegularImportsIndex,
    kModuleRequestPositionsIndex,
    kLength
  };
  enum {
    kRegularExportLocalNameOffset,
    kRegularExportCellIndexOffset,
    kRegularExportExportNamesOffset,
    kRegularExportLength
  };

  OBJECT_CONSTRUCTORS(SourceTextModuleInfo, FixedArray);
};

class SourceTextModuleInfoEntry : public Struct {
 public:
  DECL_CAST(SourceTextModuleInfoEntry)
  DECL_PRINTER(SourceTextModuleInfoEntry)
  DECL_VERIFIER(SourceTextModuleInfoEntry)

  DECL_ACCESSORS(export_name, Object)
  DECL_ACCESSORS(local_name, Object)
  DECL_ACCESSORS(import_name, Object)
  DECL_INT_ACCESSORS(module_request)
  DECL_INT_ACCESSORS(cell_index)
  DECL_INT_ACCESSORS(beg_pos)
  DECL_INT_ACCESSORS(end_pos)

  static Handle<SourceTextModuleInfoEntry> New(
      Isolate* isolate, Handle<Object> export_name, Handle<Object> local_name,
      Handle<Object> import_name, int module_request, int cell_index,
      int beg_pos, int end_pos);

  DEFINE_FIELD_OFFSET_CONSTANTS(
      Struct::kHeaderSize,
      TORQUE_GENERATED_SOURCE_TEXT_MODULE_INFO_ENTRY_FIELDS)

  OBJECT_CONSTRUCTORS(SourceTextModuleInfoEntry, Struct);
};

}  // namespace internal
}  // namespace v8

#include "src/objects/object-macros-undef.h"

#endif  // V8_OBJECTS_SOURCE_TEXT_MODULE_H_
