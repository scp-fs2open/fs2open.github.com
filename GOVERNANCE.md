# OFP Governance

This document describes how the Open FreeSpace Project makes decisions, manages its contributor ladder, and handles conflict.

---

## Principles

- Trust and authority are earned through demonstrated contribution, not granted socially.
- All decisions of consequence are made publicly and with documented reasoning.
- The Community Queue represents accumulated community need. It can be thought of as a roadmap.

---

## Decision Making

Most decisions happen in the open: PR reviews, Discussion threads, RFC discussions. The default process is lazy consensus. If no one objects with a documented reason within a reasonable window, the decision stands.

Contested decisions escalate to a Maintainer vote in the relevant Discussion thread. Maintainers vote publicly. Majority rules. Ties go to community vote (any active Contributor or above may participate).

---

## RFCs

Significant architectural changes, governance amendments, or anything that would affect the mod ecosystem broadly requires an RFC (Request for Comments) opened as a GitHub Discussion. RFCs stay open for a minimum of two weeks before a ruling. Maintainers are expected to participate and document their reasoning.

---

## Release Process

OFP targets a regular release cadence. Proposed timelines are posted as GitHub Discussions at least six weeks before a feature freeze. Community members may comment on scope, timing, and what gets held versus shipped. Maintainers make final calls but explain decisions that go against stated community preference.

Releases include:
- Features from the Community Queue that are merged and stable
- Bug fixes
- Upstream FSO merges that have passed integration testing

---

## Upstream Sync

OFP merges from FSO upstream weekly. Maintainers may defer a specific week's merge if upstream is mid-refactor, with a note posted in the sync thread. Deferred merges are picked up the following week.

Changes that conflict with OFP's architectural decisions are evaluated case by case. The decision to accept, adapt, or exclude an upstream change is documented publicly in the sync thread.

---

## Amending This Document

Governance changes require an RFC with at least two weeks of open comment and consensus among active Maintainers.
