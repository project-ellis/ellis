# Submitting patches
To submit patches to Ellis, you should use the following workflow:

- Fork the Ellis repo.
- In your forked repo, create a new branch from master, ideally with a name
  describing the types of change you want to make.
- Make your changes in whatever form you find most comfortable.
- Prior to submission, fetch the latest master and ma
- Test your changes, making sure they pass the [Quality](#Quality) tests.
- Prior to submission, fetch the latest master, rebase onto it, and retest to
  verify nothing broke.
- Squash your changes into a linear series of clean, easy-to-read commits. Break
  large commits into small, self-contained functional units. Generally, if a
  commit can be made smaller, it's a good idea to do so. A notable exception is
  adding new encoders or decoders, which can be done with one large commit.
- Make the commit subject line clearly identify the part of Ellis that is being
  changed. Although it is not strictly required, one easy way to do this is to
  add a prefix to the commit description that indicates the part of the source
  tree being touched. For example, *codec/msgpack: code cleanup*.
- If you have fixed a bug, add the text *Fixes #XXX* as a single line in its own
  paragraph in the commit description. This causes Github to automatically close
  the issue with a link to the commit.
- Open a Github pull request. If someone doesn't respond to you within a few
  days, feel free to ping the reviewers; we won't be upset :).

Note that for large changes or new design, it's a good idea to open an Issue
before starting substantial work to make sure your idea will be accepted before
you begin work. This isn't strictly necessary, but it's helpful so that you
avoid having to rewrite a large amount of code due to design disagreement.

# Quality
All new commits should meet the following:
- Code should follow the coding standards in `doc/coding_standard.md`.
- Code should not break the unit tests. Specifically, the following targets
  should succeed:
  ```
  ninja test
  ninja test-valgrind
  ninja check # static analysis
  ```
- Code that changes behavior should add or modify test coverage. If you fix a
  bug, you should add a unit test that exposes the bug; this unit test should
  fail without your bug fix. If you add a new feature, you should add a unit
  test that exercises your feature. Code refactoring generally does not unit
  tests.

# Example commit
codec/msgpack: fix msgpack encoder race

There was a race condition in the msgpack encoder triggered by encoding two
msgpack blobs at the same time and causing a crash. Fix the race by adding a
mutex.

Fixes #17.
