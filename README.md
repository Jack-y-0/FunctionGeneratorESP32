# FunctionGeneratorESP32

This repository contains the original standalone waveform sketch and the web-controlled firmware in `functiongenerator_web/`.

The `feature/multichannel-scope` branch targets the classic ESP32 DevKit (`esp32dev`). It combines two independently controlled DAC outputs with six ADC1 analog scope channels and 17 configurable digital PWM edge-capture inputs. The scope view selects ADC or PWM inputs and overlays selected channels on a full-width graph. The generator remains organized as independent DAC channel cards with one preview per channel. Both DAC outputs start disabled after reboot.

See [`functiongenerator_web/README.md`](functiongenerator_web/README.md) for the pin lists, sampling behavior, electrical limits, Wi-Fi setup, and upload instructions. The original `.ino` sketches remain standalone examples.

## Automatic versioning

[`VERSION`](VERSION) is the source of truth for the project version, initially
`0.0.7`. The Arduino sketch prints it at startup; the web firmware exposes it as
`firmware` in `/api/state` and displays it beside the connection status.

After each pull request is merged into this repository's `main` branch, GitHub
Actions increments the patch number (for example, `0.0.7` → `0.0.8`) and commits
the updated `VERSION` and both firmware identifiers to `main`. Closed, unmerged
PRs and direct pushes do not trigger a bump. Re-running a successful merge job
does not bump the same PR twice. Concurrent merges retry against the latest
`main` to preserve each increment.

The workflow requires GitHub Actions to be enabled and its `GITHUB_TOKEN` to
have permission to push to `main`. Branch protection or rulesets that prohibit
bot pushes must allow this workflow's version commit. A rejected push fails the
job without changing the remote version; rerun it after resolving permissions.
The workflow does not create Git tags or GitHub Releases. Devices show the new
version after the updated firmware is built and uploaded.

Run the versioning checks locally with:

```sh
python3 -m unittest discover -s tests -v
```

