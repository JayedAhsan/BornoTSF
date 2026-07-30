#include "mim_engine.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace khipro {

namespace {
int ParseIntOrDefault(const std::string& s, int fallback) {
  try {
    size_t consumed = 0;
    int v = std::stoi(s, &consumed);
    if (consumed != s.size()) {
      return fallback;
    }
    return v;
  } catch (...) {
    return fallback;
  }
}

}  // namespace

KhiproEngine::Parser::Parser(std::string text) : text_(std::move(text)) {}

std::vector<KhiproEngine::Sexp> KhiproEngine::Parser::ParseAll() {
  std::vector<Sexp> out;
  while (SkipWs()) {
    out.push_back(ReadExpr());
  }
  return out;
}

bool KhiproEngine::Parser::SkipWs() {
  while (idx_ < text_.size()) {
    if (std::isspace(static_cast<unsigned char>(text_[idx_]))) {
      ++idx_;
      continue;
    }
    if (text_[idx_] == ';' && idx_ + 1 < text_.size() && text_[idx_ + 1] == ';') {
      while (idx_ < text_.size() && text_[idx_] != '\n') {
        ++idx_;
      }
      continue;
    }
    return true;
  }
  return false;
}

KhiproEngine::Sexp KhiproEngine::Parser::ReadExpr() {
  if (!SkipWs()) {
    throw std::runtime_error("Unexpected EOF");
  }
  if (text_[idx_] == '(') {
    ++idx_;
    Sexp list_node;
    list_node.is_atom = false;
    while (SkipWs() && text_[idx_] != ')') {
      list_node.list.push_back(ReadExpr());
    }
    if (idx_ >= text_.size() || text_[idx_] != ')') {
      throw std::runtime_error("Unclosed list");
    }
    ++idx_;
    return list_node;
  }
  if (text_[idx_] == '"') {
    return Sexp{true, ReadString(), {}};
  }
  return Sexp{true, ReadAtom(), {}};
}

std::string KhiproEngine::Parser::ReadString() {
  std::string out;
  ++idx_;
  while (idx_ < text_.size() && text_[idx_] != '"') {
    out.push_back(text_[idx_]);
    ++idx_;
  }
  if (idx_ >= text_.size()) {
    throw std::runtime_error("Unterminated string");
  }
  ++idx_;
  return out;
}

std::string KhiproEngine::Parser::ReadAtom() {
  const size_t start = idx_;
  while (idx_ < text_.size() && !std::isspace(static_cast<unsigned char>(text_[idx_])) && text_[idx_] != '(' &&
         text_[idx_] != ')') {
    ++idx_;
  }
  return text_.substr(start, idx_ - start);
}

KhiproEngine::KhiproEngine(std::string spec_text) {
  Parser parser(std::move(spec_text));
  Build(parser.ParseAll());
  Reset();
}

void KhiproEngine::Build(const std::vector<Sexp>& root) {
  for (const auto& node : root) {
    if (node.is_atom || node.list.empty() || !node.list[0].is_atom) {
      continue;
    }
    const std::string& head = node.list[0].atom;
    if (head == "map") {
      ParseMaps(std::vector<Sexp>(node.list.begin() + 1, node.list.end()));
    } else if (head == "state") {
      ParseStates(std::vector<Sexp>(node.list.begin() + 1, node.list.end()));
    }
  }
}

void KhiproEngine::ParseMaps(const std::vector<Sexp>& nodes) {
  for (const auto& map_node : nodes) {
    if (map_node.is_atom || map_node.list.empty() || !map_node.list[0].is_atom) {
      continue;
    }
    const std::string map_name = map_node.list[0].atom;
    auto& out_entries = maps_[map_name];

    for (size_t i = 1; i < map_node.list.size(); ++i) {
      const auto& entry = map_node.list[i];
      if (entry.is_atom || entry.list.empty()) {
        continue;
      }

      std::string key;
      const auto& key_node = entry.list[0];
      if (key_node.is_atom) {
        key = key_node.atom;
      } else if (!key_node.list.empty() && key_node.list[0].is_atom) {
        key = key_node.list[0].atom == "BackSpace" ? "\b" : key_node.list[0].atom;
      }
      if (key.empty()) {
        continue;
      }

      MapEntry e;
      e.key = key;
      std::string output;
      for (size_t j = 1; j < entry.list.size(); ++j) {
        const auto& item = entry.list[j];
        if (item.is_atom) {
          output = item.atom;
          continue;
        }
        bool ok = false;
        Action a = ParseAction(item, &ok);
        if (ok) {
          e.actions.push_back(a);
        }
      }
      if (!output.empty()) {
        Action insert;
        insert.type = ActionType::Insert;
        insert.s1 = output;
        e.actions.push_back(insert);
      }
      out_entries.push_back(std::move(e));
    }

    std::sort(out_entries.begin(), out_entries.end(), [](const MapEntry& a, const MapEntry& b) {
      return a.key.size() > b.key.size();
    });
  }
}

void KhiproEngine::ParseStates(const std::vector<Sexp>& nodes) {
  for (const auto& state_node : nodes) {
    if (state_node.is_atom || state_node.list.empty() || !state_node.list[0].is_atom) {
      continue;
    }

    StateDef def;
    def.name = state_node.list[0].atom;
    for (size_t i = 1; i < state_node.list.size(); ++i) {
      const auto& entry = state_node.list[i];
      if (entry.is_atom || entry.list.empty() || !entry.list[0].is_atom) {
        continue;
      }
      const std::string head = entry.list[0].atom;
      if (head == "t") {
        for (size_t j = 1; j < entry.list.size(); ++j) {
          bool ok = false;
          Action a = ParseAction(entry.list[j], &ok);
          if (ok) {
            def.entry_actions.push_back(a);
          }
        }
      } else {
        StateRule r;
        r.map_name = head;
        for (size_t j = 1; j < entry.list.size(); ++j) {
          bool ok = false;
          Action a = ParseAction(entry.list[j], &ok);
          if (ok) {
            r.actions.push_back(a);
          }
        }
        def.rules.push_back(std::move(r));
      }
    }
    states_[def.name] = std::move(def);
  }
}

Action KhiproEngine::ParseAction(const Sexp& node, bool* ok) const {
  *ok = false;
  Action out;
  if (node.is_atom || node.list.empty() || !node.list[0].is_atom) {
    return out;
  }

  const std::string& head = node.list[0].atom;
  if (head == "set") {
    if (node.list.size() >= 3 && node.list[1].is_atom && node.list[2].is_atom) {
      out.type = ActionType::Set;
      out.s1 = node.list[1].atom;
      out.s2 = node.list[2].atom;
      *ok = true;
    }
    return out;
  }
  if (head == "insert") {
    if (node.list.size() >= 2 && node.list[1].is_atom) {
      out.type = ActionType::Insert;
      out.s1 = node.list[1].atom;
      if (!out.s1.empty() && out.s1[0] == '?') {
        out.s1.erase(0, 1);
      }
      *ok = true;
    }
    return out;
  }
  if (head == "delete") {
    if (node.list.size() >= 2 && node.list[1].is_atom) {
      const std::string token = node.list[1].atom;
      if (token.rfind("@-", 0) == 0) {
        out.type = ActionType::Delete;
        const std::string count = token.substr(2);
        out.n = count.empty() ? 1 : ParseIntOrDefault(count, 1);
        *ok = true;
      }
    }
    return out;
  }
  if (head == "move") {
    if (node.list.size() >= 2 && node.list[1].is_atom) {
      const std::string token = node.list[1].atom;
      out.type = ActionType::Move;
      if (token == "@>") {
        out.flag = true;
        out.n = 0;
        *ok = true;
      } else if (token == "@-") {
        out.n = -1;
        *ok = true;
      } else if (token.rfind("@-", 0) == 0) {
        out.n = -ParseIntOrDefault(token.substr(2), 1);
        *ok = true;
      }
    }
    return out;
  }
  if (head == "shift") {
    if (node.list.size() >= 2 && node.list[1].is_atom) {
      out.type = ActionType::Shift;
      out.s1 = node.list[1].atom;
      *ok = true;
    }
    return out;
  }
  if (head == "commit") {
    out.type = ActionType::Commit;
    *ok = true;
    return out;
  }
  if (head == "undo") {
    out.type = ActionType::Undo;
    *ok = true;
    return out;
  }
  if (head == "cond") {
    Action cond_action;
    cond_action.type = ActionType::CondBranch;

    for (size_t i = 1; i < node.list.size(); ++i) {
      const auto& branch = node.list[i];
      if (branch.is_atom || branch.list.empty()) {
        continue;
      }
      CondBranch cb;
      if (branch.list[0].is_atom && branch.list[0].atom == "1") {
        cb.condition.type = ConditionType::Always;
      } else {
        auto cond = ParseCondition(branch.list[0]);
        if (!cond) {
          continue;
        }
        cb.condition = *cond;
      }

      for (size_t j = 1; j < branch.list.size(); ++j) {
        bool child_ok = false;
        Action child = ParseAction(branch.list[j], &child_ok);
        if (child_ok) {
          cb.actions.push_back(std::move(child));
        }
      }
      cond_action.branches.push_back(std::move(cb));
    }

    if (!cond_action.branches.empty()) {
      *ok = true;
      return cond_action;
    }
  }

  return out;
}

std::shared_ptr<Condition> KhiproEngine::ParseCondition(const Sexp& node) const {
  if (node.is_atom || node.list.empty() || !node.list[0].is_atom) {
    return nullptr;
  }
  auto out = std::make_shared<Condition>();
  const std::string& head = node.list[0].atom;

  if (head == "1") {
    out->type = ConditionType::Always;
    return out;
  }
  if (head == "=") {
    if (node.list.size() >= 3 && node.list[1].is_atom && node.list[2].is_atom) {
      out->type = ConditionType::Equals;
      out->variable = node.list[1].atom;
      out->value = ParseIntOrDefault(node.list[2].atom, 0);
      return out;
    }
    return nullptr;
  }
  if (head == "&" || head == "|") {
    if (node.list.size() >= 3) {
      auto left = ParseCondition(node.list[1]);
      auto right = ParseCondition(node.list[2]);
      if (!left || !right) {
        return nullptr;
      }
      out->type = (head == "&") ? ConditionType::And : ConditionType::Or;
      out->left = left;
      out->right = right;
      return out;
    }
  }

  return nullptr;
}

void KhiproEngine::Reset() {
  current_state_ = "init";
  vars_.clear();
  ApplyEntryActions();
}

std::pair<const MapEntry*, const StateRule*> KhiproEngine::FindMatch(const StateDef& state, const std::string& input,
                                                                      size_t index) const {
  const MapEntry* best_entry = nullptr;
  const StateRule* best_rule = nullptr;
  size_t best_len = 0;

  for (const auto& rule : state.rules) {
    auto it = maps_.find(rule.map_name);
    if (it == maps_.end()) {
      continue;
    }
    for (const auto& entry : it->second) {
      if (entry.key.size() <= best_len) {
        continue;
      }
      if (index + entry.key.size() <= input.size() && input.compare(index, entry.key.size(), entry.key) == 0) {
        best_entry = &entry;
        best_rule = &rule;
        best_len = entry.key.size();
      }
    }
  }
  return {best_entry, best_rule};
}

std::string KhiproEngine::Convert(const std::string& input) {
  Reset();

  std::u32string out;
  size_t cursor = 0;
  size_t i = 0;

  while (i < input.size()) {
    auto state_it = states_.find(current_state_);
    if (state_it == states_.end()) {
      break;
    }

    auto [entry, rule] = FindMatch(state_it->second, input, i);
    if (!entry || !rule) {
      unsigned char b0 = static_cast<unsigned char>(input[i]);
      size_t len = 1;
      if ((b0 & 0xE0) == 0xC0 && i + 1 < input.size()) {
        len = 2;
      } else if ((b0 & 0xF0) == 0xE0 && i + 2 < input.size()) {
        len = 3;
      } else if ((b0 & 0xF8) == 0xF0 && i + 3 < input.size()) {
        len = 4;
      }
      std::u32string cp = Utf8ToU32(input.substr(i, len));
      if (!cp.empty()) {
        out.insert(out.begin() + static_cast<std::ptrdiff_t>(cursor), cp[0]);
        ++cursor;
      }
      i += len;
      if (current_state_ != "init") {
        current_state_ = "init";
        ApplyEntryActions();
      }
      continue;
    }

    ExecuteActions(entry->actions, &out, &cursor);
    ExecuteActions(rule->actions, &out, &cursor);
    cursor = std::min(cursor, out.size());
    i += entry->key.size();
  }

  return U32ToUtf8(out);
}

void KhiproEngine::ApplyEntryActions() {
  auto it = states_.find(current_state_);
  if (it == states_.end()) {
    return;
  }
  std::u32string dummy_out;
  size_t dummy_cursor = 0;
  ExecuteActions(it->second.entry_actions, &dummy_out, &dummy_cursor);
}

void KhiproEngine::ExecuteActions(const std::vector<Action>& actions, std::u32string* out, size_t* cursor) {
  for (const auto& action : actions) {
    switch (action.type) {
      case ActionType::Set: {
        int value = ParseIntOrDefault(action.s2, 0);
        auto it = vars_.find(action.s2);
        if (it != vars_.end()) {
          value = it->second;
        }
        vars_[action.s1] = value;
        break;
      }
      case ActionType::Insert: {
        std::u32string text = Utf8ToU32(action.s1);
        out->insert(out->begin() + static_cast<std::ptrdiff_t>(*cursor), text.begin(), text.end());
        *cursor += text.size();
        break;
      }
      case ActionType::Delete: {
        if (*cursor == 0 || action.n <= 0) {
          break;
        }
        size_t n = static_cast<size_t>(action.n);
        size_t start = (*cursor > n) ? (*cursor - n) : 0;
        if (start < *cursor && start < out->size()) {
          size_t end = std::min(*cursor, out->size());
          out->erase(out->begin() + static_cast<std::ptrdiff_t>(start),
                     out->begin() + static_cast<std::ptrdiff_t>(end));
          *cursor = start;
        }
        break;
      }
      case ActionType::Move: {
        if (action.flag) {
          *cursor = out->size();
        } else {
          const int next = static_cast<int>(*cursor) + action.n;
          *cursor = static_cast<size_t>(std::clamp(next, 0, static_cast<int>(out->size())));
        }
        break;
      }
      case ActionType::Shift: {
        current_state_ = action.s1;
        ApplyEntryActions();
        break;
      }
      case ActionType::Commit:
        break;
      case ActionType::Undo: {
        if (*cursor > 0 && !out->empty()) {
          const size_t at = *cursor - 1;
          out->erase(out->begin() + static_cast<std::ptrdiff_t>(at));
          *cursor = at;
        }
        break;
      }
      case ActionType::Cond: {
        if (EvalCond(action.condition)) {
          ExecuteActions(action.actions, out, cursor);
        }
        break;
      }
      case ActionType::CondBranch: {
        for (const auto& branch : action.branches) {
          if (EvalCond(branch.condition)) {
            ExecuteActions(branch.actions, out, cursor);
            break;
          }
        }
        break;
      }
    }
  }
}

bool KhiproEngine::EvalCond(const Condition& cond) const {
  switch (cond.type) {
    case ConditionType::Always:
      return true;
    case ConditionType::Equals: {
      auto it = vars_.find(cond.variable);
      return (it != vars_.end() ? it->second : 0) == cond.value;
    }
    case ConditionType::And:
      return cond.left && cond.right && EvalCond(*cond.left) && EvalCond(*cond.right);
    case ConditionType::Or:
      return cond.left && cond.right && (EvalCond(*cond.left) || EvalCond(*cond.right));
  }
  return false;
}

std::u32string KhiproEngine::Utf8ToU32(const std::string& s) {
  std::u32string out;
  size_t i = 0;
  while (i < s.size()) {
    char32_t cp = 0;
    unsigned char b = static_cast<unsigned char>(s[i]);
    if (b < 0x80) {
      cp = b;
      ++i;
    } else if ((b & 0xE0) == 0xC0 && i + 1 < s.size()) {
      cp = (static_cast<char32_t>(b & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
      i += 2;
    } else if ((b & 0xF0) == 0xE0 && i + 2 < s.size()) {
      cp = (static_cast<char32_t>(b & 0x0F) << 12) |
           (static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6 |
           (static_cast<unsigned char>(s[i + 2]) & 0x3F);
      i += 3;
    } else if ((b & 0xF8) == 0xF0 && i + 3 < s.size()) {
      cp = (static_cast<char32_t>(b & 0x07) << 18) |
           (static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12 |
           (static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6 |
           (static_cast<unsigned char>(s[i + 3]) & 0x3F);
      i += 4;
    } else {
      ++i;
      continue;
    }
    out.push_back(cp);
  }
  return out;
}

std::string KhiproEngine::U32ToUtf8(const std::u32string& s) {
  std::string out;
  for (char32_t cp : s) {
    if (cp < 0x80) {
      out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
      out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
      out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
      out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
  }
  return out;
}

bool PopLastUtf8Codepoint(std::string* s) {
  if (!s || s->empty()) {
    return false;
  }
  size_t i = s->size() - 1;
  while (i > 0 && (((*s)[i] & 0xC0) == 0x80)) {
    --i;
  }
  s->erase(i);
  return true;
}

}  // namespace khipro
