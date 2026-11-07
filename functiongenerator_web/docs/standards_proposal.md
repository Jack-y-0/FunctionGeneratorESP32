# Standards proposal — awaiting approval

Research date: 2026-11-08. These are candidates for the ESP32 web-controlled function generator. No standard is adopted by this document, and no conformity or certification is claimed.

## Assumed intended use

A USB-powered, low-frequency, two-channel bench signal generator with nominal 0–3.3 V DAC outputs, connected to compatible high-impedance inputs. The synthetic PPG shape alone does not establish a medical-device intended use. Confirm intended use, hardware configuration, environment, and target market before deciding formal applicability.

## Candidates

| Candidate | Why it may fit | Proposed work after approval | Limits |
|---|---|---|---|
| IEC 61010-1:2010, including Amendment 1:2016 | General safety requirements for electrical test, measurement, control, and laboratory equipment; the proposed bench instrument falls within that broad equipment category. | Review the complete hardware and power arrangement; prepare an applicability and gap assessment, then define safety requirements and verification evidence. | Firmware changes alone cannot establish conformity. Full normative text, relevant amendments/particular standards, and qualified hardware assessment are needed. |
| IEC 61326-1:2020 | EMC emissions and immunity requirements for measurement, control, and laboratory equipment. | Define the intended electromagnetic environment, cable/setup configuration, performance criteria, and an EMC verification plan for the assembled instrument. | Requires hardware/system testing; a build or web preview test is not EMC evidence. |
| Semantic Versioning 2.0.0 | A software release convention for communicating compatibility through MAJOR.MINOR.PATCH versions. | Define the public interface (HTTP API, serial commands, persisted settings compatibility), choose an initial development version, and align release tags, firmware-reported version, and history. | This is a release convention, not an electrical safety or medical compliance standard. No version number has been assigned by this proposal. |

## Recommendation for approval

Use IEC 61010-1 as the primary candidate for a bench-instrument safety gap assessment, with IEC 61326-1 as the companion EMC candidate. Adopt Semantic Versioning separately if numbered software releases are wanted.

Approval would authorize an initial assessment and release-process implementation, not a declaration of conformity. A clause-level assessment needs the applicable full standards; the public scope descriptions reviewed here do not provide enough detail for one. Medical-device standards require a separate intended-use assessment if patient connection or clinical use is planned.

## Sources reviewed

- [IEC 61010-1:2010 — official scope](https://webstore.iec.ch/en/publication/4279)
- [IEC 61010-1 Amendment 1:2016 — official listing](https://webstore.iec.ch/en/publication/33314)
- [IEC 61326-1:2020 — official scope](https://webstore.iec.ch/en/publication/62793)
- [Semantic Versioning 2.0.0 — specification](https://semver.org/spec/v2.0.0.html)
