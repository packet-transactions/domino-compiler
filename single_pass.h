#ifndef SINGLE_PASS_H_
#define SINGLE_PASS_H_

// Most code here is based on http://eli.thegreenplace.net/2012/06/08/basic-source-to-source-transformation-with-clang

#include <functional>
#include <cstdio>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Support/Host.h"

#include "clang_utility_functions.h"

/// Single pass over a translation unit.
/// By default, just parse the translation unit, and print it out as such.
template <class OutputType>
class SinglePass {
 public:
  typedef std::function<OutputType(const clang::TranslationUnitDecl *)> Transformer;

  /// Run a single pass over a given filename
  /// Using the given Transformer object
  SinglePass(const std::string & file_name,
             const Transformer & t_transformer);

  /// Output from SinglePass
  auto output() const { return output_; }
 private:
  class MyASTConsumer : public clang::ASTConsumer {
    public:
    MyASTConsumer(const Transformer & t_transformer) : transformer_(t_transformer) {};

    /// Override the method that gets called for the translation unit
    virtual void HandleTranslationUnit(clang::ASTContext & context) override {
      const auto * tu_decl = context.getTranslationUnitDecl();
      assert(llvm::isa<clang::TranslationUnitDecl>(tu_decl));
      output_ = transformer_(tu_decl);
    }
    auto output() const { return output_; }
    private:
      OutputType output_ = {};
      /// Transformer function
      Transformer transformer_;
  };
  /// Instantiate MyASTConsumer using supplied transformer
  MyASTConsumer my_ast_consumer_;

  /// Output Type
  OutputType output_ = {};
};

template <class OutputType>
SinglePass<OutputType>::SinglePass(const std::string & file_name,
                                   const Transformer & t_transformer)
    : my_ast_consumer_(t_transformer) {
  // clang::CompilerInstance will hold the instance of the Clang compiler for us,
  // managing the various objects needed to run the compiler.
  clang::CompilerInstance TheCompInst;
  TheCompInst.createDiagnostics();
  TheCompInst.getLangOpts().CPlusPlus = 0;

  // Initialize target info with the default triple for our platform.
  auto TO = std::make_shared<clang::TargetOptions>();
  TO->Triple = llvm::sys::getDefaultTargetTriple();
  clang::TargetInfo *TI =
      clang::TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
  TheCompInst.setTarget(TI);

  TheCompInst.createFileManager();
  clang::FileManager &FileMgr = TheCompInst.getFileManager();
  TheCompInst.createSourceManager(FileMgr);
  clang::SourceManager &SourceMgr = TheCompInst.getSourceManager();
  TheCompInst.createPreprocessor(clang::TU_Module);
  TheCompInst.createASTContext();

  // Set the main file handled by the source manager to the input file.
  const clang::FileEntry *FileIn = FileMgr.getFile(file_name.c_str());
  SourceMgr.setMainFileID(
      SourceMgr.createFileID(FileIn, clang::SourceLocation(), clang::SrcMgr::C_User));
  TheCompInst.getDiagnosticClient().BeginSourceFile(
      TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

  // Parse the file to AST, registering my_ast_consumer_ as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), &my_ast_consumer_,
           TheCompInst.getASTContext());

  output_ = my_ast_consumer_.output();
}

#endif  // SINGLE_PASS_H_
