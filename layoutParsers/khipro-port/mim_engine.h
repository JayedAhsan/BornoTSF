#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace khipro {

enum class ConditionType {
  Always,
  Equals,
  And,
  Or,
};

struct Condition {
  ConditionType type = ConditionType::Always;
  std::string variable;
  int value = 0;
  std::shared_ptr<Condition> left;
  std::shared_ptr<Condition> right;
};

enum class ActionType {
  Set,
  Insert,
  Delete,
  Move,
  Shift,
  Commit,
  Cond,
  CondBranch,
  Undo,
};

struct Action;

struct CondBranch {
  Condition condition;
  std::vector<Action> actions;
};

struct Action {
  ActionType type = ActionType::Commit;
  std::string s1;
  std::string s2;
  int n = 0;
  bool flag = false;
  Condition condition;
  std::vector<Action> actions;
  std::vector<CondBranch> branches;
};

struct MapEntry {
  std::string key;
  std::vector<Action> actions;
};

struct StateRule {
  std::string map_name;
  std::vector<Action> actions;
};

struct StateDef {
  std::string name;
  std::vector<Action> entry_actions;
  std::vector<StateRule> rules;
};

class KhiproEngine {
 public:
  explicit KhiproEngine(std::string spec_text);

  void Reset();
  std::string Convert(const std::string& input);

 private:
  using Vars = std::unordered_map<std::string, int>;

  struct Sexp {
    bool is_atom = true;
    std::string atom;
    std::vector<Sexp> list;
  };

  class Parser {
   public:
    explicit Parser(std::string text);
    std::vector<Sexp> ParseAll();

   private:
    bool SkipWs();
    Sexp ReadExpr();
    std::string ReadString();
    std::string ReadAtom();

    std::string text_;
    size_t idx_ = 0;
  };

  void Build(const std::vector<Sexp>& root);
  void ParseMaps(const std::vector<Sexp>& nodes);
  void ParseStates(const std::vector<Sexp>& nodes);

  Action ParseAction(const Sexp& node, bool* ok) const;
  std::shared_ptr<Condition> ParseCondition(const Sexp& node) const;

  void ApplyEntryActions();
  std::pair<const MapEntry*, const StateRule*> FindMatch(const StateDef& state, const std::string& input,
                                                         size_t index) const;

  void ExecuteActions(const std::vector<Action>& actions, std::u32string* out, size_t* cursor);
  bool EvalCond(const Condition& cond) const;

  static std::u32string Utf8ToU32(const std::string& s);
  static std::string U32ToUtf8(const std::u32string& s);

  std::unordered_map<std::string, std::vector<MapEntry>> maps_;
  std::unordered_map<std::string, StateDef> states_;
  std::string current_state_ = "init";
  Vars vars_;
};

bool PopLastUtf8Codepoint(std::string* s);

}  // namespace khipro
