# Git Commit Rules

From now on, every commit message must be short, professional, and follow Conventional Commits.

Examples:
- `feat: scheduler init`
- `feat: task registry`
- `feat: interrupt profiler`
- `feat: tinyml runtime`
- `feat: benchmark logger`
- `fix: cmocka tests`
- `fix: scheduler lookup`
- `ci: update workflows`
- `docs: api reference`
- `test: scheduler registry`

## Rules:
- Maximum ~50 characters whenever practical.
- Do NOT write long descriptive commit titles.
- Do NOT include implementation details in the title.
- Use the commit body only if extra explanation is needed.
- During feature development there must be exactly ONE feature commit.
- Any CI fixes must be added using `git commit --amend --no-edit` and `git push --force-with-lease`.
- Never create commits named: "Fix CI", "Formatting", "Final fix", "Update", "Misc changes", "Try again", etc.
- Keep the public Git history clean and professional.
