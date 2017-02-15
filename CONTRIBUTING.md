# How to submit patches

#### Workflow

Use the following workflow when making changes to Ellis:

- Ideally, begin with creating a github issue, especially
  for bug fixes (though also encouraged for new features).
- It's a good idea to discuss briefly on the issues page, to make sure the
  plan is approved, before embarking on substantial work.  It's not strictly
  necessary, but it's helpful for reducing wastage and disagreement.
- Fork the repo.
- In your fork, create a new branch from master, ideally with a name
  describing the types of change you want to make.
- Make your commits as desired.  (See below re cleanup/rebasing).
- Test your changes, adhering to [code quality guidelines](#Quality-guidelines)
  and testing.
- Prior to submission, fetch the latest master, rebase onto it, and retest to
  verify nothing broke.
- Squash your changes into a linear series of clean, small, self-contained,
  and easy-to-read commits.  When in doubt, smaller commits are typically
  preferred. A notable exception is adding new encoders or decoders, which can
  be done with one large commit.  Please follow the
  [guidelines for commit messages](#Commit-messages).
- Open a Github pull request. If someone doesn't respond to you within a few
  days, feel free to ping the reviewers; they won't mind. :).

# Quality guidelines

* Follow the [coding standards](doc/coding_standard.md).
* Do not break the unit tests.

After cleaning up and rebasing commits, each commit should pass:

```
ninja test
ninja test-valgrind
ninja check # static analysis
```

If you add a feature, *include a unit test*.

If you fix a bug, *add a unit test* that would have caught the bug.

If you are changing code behavior, *modify unit tests* appropriately.

Unit tests should accompany most everything that's not refactoring.

# Commit messages

#### Title

The commit title string is lowercase, with no period at the end.  Try to stay
under 50 chars (target), and never exceed 72 (hard limit).

First identify the part of Ellis that is changed, and then summarize the
change starting with a verb in the imperative grammatical form--for example,
`codec/msgpack: remove dead code`--so that it is quick for a human reader to
parse.

Leave a blank line between the title and the body of the commit message.

#### Body

Here, you should explain the crucial information that may not stand out just
from looking at the code.  This might be an explanation for the overall focus
of the changes, or perhaps of why the changes are made in that way.

#### Automatic resolution

When you are resolving an ellis issue recorded via github, use the string
`"Resolves #XXX"` (where XXX is the issue number).

There are other synonyms that cause automatic resolution on github, but we
like to be consistent with the word "Resolves" for the sake of searching.

This goes on its own line.

#### Example message

```
codec/msgpack: fix msgpack encoder race

There was a race condition in the msgpack encoder triggered by encoding two
msgpack blobs at the same time and causing a crash. Fix the race by adding a
mutex.

Resolves #17.
```
