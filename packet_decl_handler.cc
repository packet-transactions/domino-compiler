#include "clang_utility_functions.h"
#include "packet_decl_handler.h"

using namespace clang;
using namespace clang::ast_matchers;

void PacketDeclHandler::run(const MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<Decl>("decl");
  assert(decl != nullptr);

  // Handle only translation unit decls
  if (isa<TranslationUnitDecl>(decl)) {
    // Get all decls by dyn casting decl into a DeclContext
    for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
      assert(child_decl);
      assert(child_decl->isDefinedOutsideFunctionOrMethod());
      if (isa<RecordDecl>(child_decl)) {
        // accumulate "struct Packet {" at the beginning, TODO: don't hardcode
        output_ += "typedef struct Packet {";

        // acummulate current fields in struct
        for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
          output_ += dyn_cast<ValueDecl>(field_decl)->getType().getAsString() + " " + clang_value_decl_printer(dyn_cast<ValueDecl>(field_decl)) + ";";

        // Add newly created fields
        for (const auto & new_decl : new_decls_)
          output_ += new_decl;

        // Close struct definition
        output_ += "} Packet;";
      }
    }
  }
}
