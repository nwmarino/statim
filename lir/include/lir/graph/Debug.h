//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_DEBUG_H_
#define LIR_DEBUG_H_

#include "lir/graph/Value.h"

#include <cstdint>
#include <string>
#include <vector>

namespace lir {

class CFG;

class DebugNode {
protected:
    uint32_t m_id;

    DebugNode(uint32_t id) : m_id(id) {}

public:
    virtual ~DebugNode() = default;

    DebugNode(const DebugNode&) = delete;
    void operator=(const DebugNode&) = delete;

    DebugNode(DebugNode&&) noexcept = delete;
    void operator=(DebugNode&&) noexcept = delete;

    /// Returns the id of this node.
    uint32_t id() const { return m_id; }

    /// Print this debug node in a reproducible plaintext format to |os|, 
    /// following the given printing |policy|.
    virtual void print(std::ostream &os, PrintPolicy policy) const = 0;
};

class DebugFile final : public DebugNode {
    friend class DebugBuilder;

    uint32_t m_fid;
    std::string m_path;
    std::string m_file;

    DebugFile(uint32_t id, uint32_t fid, const std::string& path, 
              const std::string& file)
      : DebugNode(id), m_fid(fid), m_path(path), m_file(file) {}

public:
    ~DebugFile() override = default;

    DebugFile(const DebugFile&) = delete;
    void operator=(const DebugFile&) = delete;

    DebugFile(DebugFile&&) noexcept = delete;
    void operator=(DebugFile&&) noexcept = delete;

    /// Returns the special file id for this debug file.
    uint32_t fid() const { return m_fid; }

    /// Returns the absolute path of this debug file.
    const std::string& path() const { return m_path; }

    /// Returns the file handle of this debug file.
    const std::string& file() const { return m_file; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

class DebugLoc final : public DebugNode {
    friend class DebugBuilder;

    DebugFile* m_file;
    uint32_t m_line;
    uint32_t m_col;

    DebugLoc(uint32_t id, DebugFile* file, uint32_t line, uint32_t col)
      : DebugNode(id), m_file(file), m_line(line), m_col(col) {}

public:
    ~DebugLoc() override = default;

    DebugLoc(const DebugLoc&) = delete;
    void operator=(const DebugLoc&) = delete;

    DebugLoc(DebugLoc&&) noexcept = delete;
    void operator=(DebugLoc&&) noexcept = delete;

    /// Returns the file debug node which this location resides in.
    const DebugFile* file() const { return m_file; }
    DebugFile* file() { return m_file; }

    /// Returns the line of this location.
    uint32_t line() const { return m_line; }

    /// Returns the column of this location.
    uint32_t col() const { return m_col; }

    void print(std::ostream &os, PrintPolicy policy) const override;
};

class DebugBuilder final {
    CFG& m_graph;
    std::vector<DebugNode*>& m_symbols;

public:
    DebugBuilder(CFG& graph);

    ~DebugBuilder() = default;

    DebugBuilder(const DebugBuilder&) = delete;
    void operator=(const DebugBuilder&) = delete;

    DebugBuilder(DebugBuilder&&) noexcept = delete;
    void operator=(DebugBuilder&&) noexcept = delete;

    /// Build a new file debug node.
    DebugFile* build_file(const std::string& path, const std::string& filename);

    /// Build a new location debug node.
    /// If |file| is not provided, then the builder will use the state file.
    DebugLoc* build_loc(DebugFile* file, uint32_t line, uint32_t col);
};

} // namespace lir

#endif // LIR_DEBUG_H_
