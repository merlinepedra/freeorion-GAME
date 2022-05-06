#ifndef _Condition_h_
#define _Condition_h_


#include <memory>
#include <string>
#include <vector>
#include "../util/Export.h"


class UniverseObject;
struct ScriptingContext;

namespace Effect {
    typedef std::vector<std::shared_ptr<UniverseObject>> TargetSet;
}

namespace Condition {

using ObjectSet = std::vector<std::shared_ptr<const UniverseObject>>;
using Mask = std::vector<unsigned char>;


enum class SearchDomain : char {
    NON_MATCHES,    ///< The Condition will only examine items in the non matches set; those that match the Condition will be inserted into the matches set.
    MATCHES         ///< The Condition will only examine items in the matches set; those that do not match the Condition will be inserted into the nonmatches set.
};

/** The base class for all Conditions. */
struct FO_COMMON_API Condition {
    virtual ~Condition() = default;

    [[nodiscard]] virtual bool operator==(const Condition& rhs) const;
    [[nodiscard]] bool operator!=(const Condition& rhs) const { return !(*this == rhs); }

    /** Tests input \a candidates a returns a vector of 1s and 0s that indicate which
      * of the input \a candidates are matched by this condition. If a non-empty \a mask
      * is passed, entries in \a candidates are skipped when the same index in \a mask
      * is 0 and the return value in those indicies is unspecified. */
    [[nodiscard]] virtual Mask Eval(const ScriptingContext& parent_context,
                                    const ObjectSet& candidates,
                                    const Mask& mask = {}) const;

    virtual void Eval(const ScriptingContext& parent_context,
                      ObjectSet& matches,
                      ObjectSet& non_matches,
                      SearchDomain search_domain = SearchDomain::NON_MATCHES) const;

    void Eval(const ScriptingContext& parent_context,
              Effect::TargetSet& matches,
              Effect::TargetSet& non_matches,
              SearchDomain search_domain = SearchDomain::NON_MATCHES) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void Eval(const ScriptingContext& parent_context, ObjectSet& matches) const;

    /** Tests all objects in universe as NON_MATCHES. */
    void Eval(const ScriptingContext& parent_context, Effect::TargetSet& matches) const;

    /** Tests single candidate object, returning true iff it matches condition. */
    [[nodiscard]] bool Eval(const ScriptingContext& parent_context,
                            std::shared_ptr<const UniverseObject> candidate) const;

    virtual void GetDefaultInitialCandidateObjects(const ScriptingContext& parent_context,
                                                   ObjectSet& condition_non_targets) const;

    /** Derived Condition classes can override this to true if all objects returned
      * by GetDefaultInitialCandidateObject() are guaranteed to also match this
      * condition. */
    [[nodiscard]] virtual bool InitialCandidatesAllMatch() const { return false; }

    //! Returns true iff this condition's evaluation does not reference
    //! the RootCandidate objects.  This requirement ensures that if this
    //! condition is a subcondition to another Condition or a ValueRef, this
    //! condition may be evaluated once and its result used to match all local
    //! candidates to that condition.
    [[nodiscard]] bool RootCandidateInvariant() const { return m_root_candidate_invariant; }

    //! (Almost) all conditions are varying with local candidates; this is the
    //! point of evaluating a condition.  This funciton is provided for
    //! consistency with ValueRef, which may not depend on the local candidiate
    //! of an enclosing condition.
    [[nodiscard]] bool LocalCandidateInvariant() const { return false; }

    //! Returns true iff this condition's evaluation does not reference the
    //! target object.
    [[nodiscard]] bool TargetInvariant() const { return m_target_invariant; }

    //! Returns true iff this condition's evaluation does not reference the
    //! source object.
    [[nodiscard]] bool SourceInvariant() const { return m_source_invariant; }

    [[nodiscard]] virtual std::string Description(bool negated = false) const = 0;
    [[nodiscard]] virtual std::string Dump(unsigned short ntabs = 0) const = 0;
    virtual void SetTopLevelContent(const std::string& content_name) = 0;
    [[nodiscard]] virtual unsigned int GetCheckSum() const { return 0; }

    //! Makes a clone of this Condition in a new owning pointer. Required for
    //! Boost.Python, which doesn't support move semantics for returned values.
    [[nodiscard]] virtual std::unique_ptr<Condition> Clone() const = 0;

protected:
    Condition() = default;
    //! Copies invariants from other Condition
    Condition(const Condition& rhs) = default;
    Condition(Condition&& rhs) = delete;
    Condition& operator=(const Condition& rhs) = delete;
    Condition& operator=(Condition&& rhs) = delete;

    bool m_root_candidate_invariant = false;
    bool m_target_invariant = false;
    bool m_source_invariant = false;

private:
    [[nodiscard]] virtual bool Match(const ScriptingContext& local_context) const; // not = 0 because some conditions aren't or can't be implemented with a single-candidate Match function
};

}


#endif
