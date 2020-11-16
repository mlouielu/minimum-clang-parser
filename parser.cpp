#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <vector>

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"

static llvm::cl::opt<std::string> fileName(llvm::cl::Positional,
                                           llvm::cl::desc("Input file"),
                                           llvm::cl::Required);

int main(int argc, char **argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv, "My simple front end\n");

  clang::CompilerInstance CI;
  clang::DiagnosticOptions DO;

  CI.createDiagnostics();

  auto Invocation = std::make_shared<clang::CompilerInvocation>();
  clang::CompilerInvocation::CreateFromArgs(
      *Invocation, llvm::makeArrayRef(argv + 1, argc - 1), CI.getDiagnostics());
  CI.setInvocation(std::move(Invocation));

  std::shared_ptr<clang::TargetOptions> pto =
      std::make_shared<clang::TargetOptions>();
  pto->Triple = llvm::sys::getDefaultTargetTriple();
  clang::TargetInfo *pti =
      clang::TargetInfo::CreateTargetInfo(CI.getDiagnostics(), pto);
  CI.setTarget(pti);

  CI.createFileManager();
  CI.createSourceManager(CI.getFileManager());

  clang::HeaderSearchOptions &HSO = CI.getHeaderSearchOpts();
  HSO.AddPath("~/llvm-project/libcxx/include", clang::frontend::System, false,
              true);
  HSO.AddPath("~/llvm-project/build/lib/clang/10.0.1/include",
              clang::frontend::System, false, true);
  HSO.AddPath("/usr/include", clang::frontend::System, false, true);

  clang::LangOptions langOpts;
  langOpts.GNUMode = 1;
  langOpts.CXXExceptions = 1;
  langOpts.RTTI = 1;
  langOpts.Bool = 1;
  langOpts.CPlusPlus = 1;
  langOpts.CPlusPlus11 = 1;
  langOpts.CPlusPlus14 = 1;

  llvm::Triple T(pto->Triple);
  Invocation->setLangDefaults(
      langOpts, clang::InputKind(clang::Language::CXX).getPreprocessed(), T,
      CI.getPreprocessorOpts(), clang::LangStandard::lang_gnucxx14);

  CI.createPreprocessor(clang::TU_Complete);
  CI.getPreprocessorOpts().UsePredefines = false;

  std::unique_ptr<clang::ASTConsumer> astConsumer =
      clang::CreateASTPrinter(NULL, "");
  CI.setASTConsumer(std::move(astConsumer));
  CI.createASTContext();
  CI.createSema(clang::TU_Complete, nullptr);

  const clang::FileEntry *file = CI.getFileManager().getFile(fileName).get();
  if (!file) {
    llvm::errs() << "File not found: " << fileName;
    return 1;
  }
  clang::FileID mainFileID = CI.getSourceManager().createFileID(
      file, clang::SourceLocation(), clang::SrcMgr::C_User);
  CI.getSourceManager().setMainFileID(mainFileID);

  CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), 0);
  clang::ParseAST(CI.getSema());
  CI.getASTContext().PrintStats();
  CI.getASTContext().Idents.PrintStats();
}
