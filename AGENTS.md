<!-- cairn:begin -->
## Roadmap and issues

This project tracks its roadmap and issues with `cairn`. Every item is a Markdown file under `cairn/items`, described by the schema in `cairn.toml`.

**Do not create ad-hoc TODO, PLAN or NOTES files.** Create a cairn item instead, so the work appears on the board and in the generated roadmap.

### The loop

1. `cairn next` — what is ready to start. It excludes anything blocked by unfinished dependencies and puts work already in progress first.
2. `cairn claim <ID>` — take it before you start, so no one duplicates the work. `cairn claim --next` picks and claims the top-ranked unclaimed item in one step, and prints its body so you can begin immediately.
3. Do the work. Record what you learn: `cairn set <ID> <field>=<value>` for fields, `cairn note <ID> "<TEXT>"` for anything that needs a sentence — why you chose something, what you tried, what to watch for.
4. `cairn close <ID>` when it is done, or `cairn release <ID>` to hand it back.
5. `cairn check` before you report finished. It must pass.

### Commands

```sh
cairn next --json                 # ready work, ranked
cairn claim --next                # take the next ready item
cairn search <TEXT> --json        # titles, bodies and labels
cairn list --json                 # all open items
cairn list --filter 'blocked=false,priority=p0'
cairn show <ID> --json            # one item, including its body
cairn new "<TITLE>" --type <TYPE> --milestone <MILESTONE>
cairn set <ID> status=<STATUS>    # also labels+=x, or any field below
cairn note <ID> "<TEXT>"          # append reasoning; never replaces
cairn close <ID>
cairn check                       # validate; run before finishing
cairn render                      # regenerate ROADMAP.md
```

### Schema

- **Types**: `optics`, `instrument`, `verify`, `prose`, `spike`, `bug`, `chore`, `milestone`
- **Statuses**: `backlog` (open), `planned` (open), `doing` (active), `blocked` (active), `done` (done), `dropped` (dropped)
- **`due`**: date, YYYY-MM-DD — when a milestone is meant to land
- **`part_of`**: names any items, by id, several allowed — a larger piece of work this belongs to
- **`priority`**: one of p0, p1, p2, p3 — p0 blocks the milestone
- **`effort`**: one of s, m, l, xl — Rough size, not an estimate
- **`area`**: one of units, spectrum, colour, geometry, accel, transport, bsdf, light, camera, sampling, media, scene, instrument, verification, prose, build — Subsystem this touches
- **Milestones**: `v0.1`, `v0.2`, `v0.3`, `v0.4`, `v0.5`, `v0.6`, `v0.7`, `v0.8`, `v0.9`, `v1.0`, `v1.1`, `v1.2`, `v1.3`, `v1.4`, `v1.5`, `later`
- **Saved views** (`cairn list --view NAME`): `now`, `next`, `thesis`, `derivations`, `honesty`, `triage`

### Rules

1. Before starting work, find or create the item and set it to an active status.
2. Use the fields above rather than inventing new ones; add new fields to `cairn.toml` first.
3. Never hand-edit the generated roadmap file — change items and run `cairn render`.
4. `cairn check` must pass before the work is considered done.

<!-- cairn:end -->

## The git loop

Everything above is about *what* to work on. This is about where the work
happens, and it exists because several agents working a backlog in one
checkout will overwrite each other's edits and interleave each other's
commits, and the log is meant to be readable.

**One item, one worktree, one branch, one commit, one pull request.**

```sh
git worktree add .claude/worktrees/0021-sphere -b 0021-sphere origin/main
cd .claude/worktrees/0021-sphere
cairn claim 21
# ... do the work ...
make && ./cornell
git add -A && git commit
.claude/propose        # rebase onto origin/main, build, push, open the PR
.claude/land           # wait for the checks, rebase-merge, remove the branch
```

Every change arrives as a pull request. The one-line ones, the prose ones,
and the ones that change this file. Not because anybody is going to review
them — this is a project of one — but because a pull request is the only
place a change is checked by something that is not the machine that wrote
it.

`propose` refuses rather than guesses: a dirty tree, a conflicted rebase, or
a rebase whose result does not build all stop it, and it says which. Run it
again after more commits and it updates the same pull request.

`land` waits for CI, merges by rebase, and removes the branch on both sides
and the worktree it was written in. It will not treat "no checks configured"
as "checks passed".

There are no merge commits. `main` is a single line of argument, and a merge
commit in a log meant to be read as prose is a paragraph that says nothing.
There are no long-lived branches either — if a branch outlives the item it
was named for, the item was too large and should have been two.

A Claude Code session can do the first two lines with the `EnterWorktree`
tool instead, which puts the worktree in the same place under the same
naming. Either way the branch is named for the item: `0021-sphere`, the id
first, so that `git branch` sorts into roadmap order.

### The two gates

`.claude/settings.json` installs `.claude/hooks/build-gate.sh` in front of
every shell command. Before any `git commit` it runs `make` and `cairn
check`, and a failure stops the commit with the compiler's own output.

This is deliberately *not* a `.git/hooks/pre-commit`. That file is untracked,
is not copied into a worktree, and therefore protects exactly the one clone
somebody remembered to install it in — which is the wrong number of clones
for a workflow whose whole shape is many worktrees at once.

It is also not the authority. `.github/workflows/ci.yml` builds every pull
request from a clean checkout, on Linux under gcc and on macOS under clang,
runs the program, and then builds it again with every warning fatal. The
local gate makes the cycle seconds instead of minutes; CI is what the claim
actually rests on, and it is the only portability test this project has.

`--no-verify` skips the local gate, spelled the way git spells it. It cannot
skip CI, which is the point of there being two.

### Where the roadmap lives while this is happening

`cairn/items/*.md` and `ROADMAP.md` both carry `merge=cairn` in
`.gitattributes`, so two branches that each added an item, or each
re-rendered the roadmap, reconcile without a person. That is the reason this
workflow can have several worktrees open on the same backlog at once. It does
not extend to the physics: two branches editing the same header is a
conflict, and it should be.

### The commit itself

House rule in `CLAUDE.md`: present tense, one subsystem, and say what the
part *does* rather than what you did to it. `propose` passes the commit
message straight through to the pull request, so the commit message is the
pull request description and there is only one thing to write.

Close the cairn item in the same commit as the work, so that the roadmap and
the code cannot disagree about what is finished.

