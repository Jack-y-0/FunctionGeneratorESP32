# Safety Gap assesment

Research date: 2026-11-08. These are candidates for the ESP32 web-controlled function generator. No standard is adopted by this document, and no conformity or certification is claimed.

## Assumed intended use

A USB-powered, low-frequency, two-channel bench signal generator with nominal 0–3.3 V DAC outputs, connected to compatible high-impedance inputs. The synthetic PPG shape alone does not establish a medical-device intended use. Confirm intended use, hardware configuration, environment, and target market before deciding formal applicability.

## Proposed standards

| Candidate | Relevance | Assessment status |
|---|---|---|
| IEC 61010-1:2010, including Amendment 1:2016 | General safety requirements for electrical test, measurement, control, and laboratory equipment. Proposed primary safety reference for this bench instrument. | Candidate only; formal applicability and clause-level gaps have not been assessed. |
| IEC 61326-1:2020 | EMC emissions and immunity requirements for measurement, control, and laboratory equipment. | Candidate only; no EMC test evidence has been established here. |
| Semantic Versioning 2.0.0 | MAJOR.MINOR.PATCH software release numbering to communicate public-interface compatibility. | Proposed release convention; not a safety or medical compliance standard. |

The work below is a proposed project work breakdown, not a clause-by-clause list of normative requirements. Both IEC candidates involve preparation that can be done without hardware and verification that requires hardware.

## Work that does not require physical hardware

These activities can be performed using source code, specifications, design records, and the applicable standards. Missing hardware design information may still block conclusions.

| Reference | Proposed activity | Expected output |
|---|---|---|
| IEC 61010-1 | Confirm intended use, users, environment, target market, and instrument boundaries; review full normative text and relevant particular standards. | Applicability record and scope decisions. |
| IEC 61010-1 | Review schematics, power-supply specifications, output interface, and available enclosure information; identify potential hazards and missing evidence. | Preliminary safety gap register and hardware questions; final physical verification remains necessary. |
| IEC 61010-1 | Review firmware behavior at startup, output enable/disable, input limits, settings persistence, and failure handling; plan verification. | Software review findings and proposed tests, with device-dependent checks listed separately. |
| IEC 61010-1 | Prepare operating limits, wiring instructions, and proposed safety verification records. | Draft user documentation and verification plan. |
| IEC 61326-1 | Define intended electromagnetic environment, representative operating modes, cable configuration, and performance criteria using available design records. | Draft EMC test configuration and plan, to be confirmed on the assembled instrument. |
| Semantic Versioning 2.0.0 | Define the public interface: HTTP API, serial commands, and persisted-settings compatibility. Select an initial development version after approval. | Versioning policy and interface compatibility definition. |
| Semantic Versioning 2.0.0 | Align Git release tags, firmware-reported version, and version history; check consistency in software builds. | Traceable software release records. |

## Work that requires hardware

These activities require the actual instrument or representative hardware, appropriate measurement equipment, and qualified review. The precise tests and acceptance limits must follow the applicable full standards and the agreed configuration.

| Reference | Proposed activity | Hardware or facilities needed | Expected evidence |
|---|---|---|---|
| IEC 61010-1 | Inspect actual power arrangement, wiring, output connections, enclosure, and protective measures against the design records. | Assembled instrument, power source, schematics, and appropriate inspection tools. | Physical inspection record and confirmed configuration. |
| IEC 61010-1 | Verify output behavior and limits, startup/reboot behavior, disable behavior, and response to relevant fault conditions. | ESP32 instrument, compatible test loads, oscilloscope or suitable measurement instruments, and a controlled test setup. | Measured results and fault-response records; simulated browser previews are insufficient. |
| IEC 61010-1 | Perform applicable electrical, thermal, mechanical, and abnormal-condition verification identified by the assessment. | Representative complete hardware and appropriate safety test facilities. | Applicable safety test reports and documented unresolved gaps. |
| IEC 61326-1 | Confirm representative cables, power supply, enclosure, connected equipment, and operating modes. | Complete assembled system and representative accessories. | Recorded EMC test setup. |
| IEC 61326-1 | Perform applicable emissions and immunity tests and monitor operation against agreed performance criteria. | Appropriate EMC laboratory/facilities and the configured instrument. | EMC test reports, observed failures, and any required design corrections. |

Semantic Versioning itself requires no physical hardware. Confirming which version is actually flashed on a board is a separate device verification activity.

## Recommendation for approval

Use IEC 61010-1 as the primary candidate for a bench-instrument safety gap assessment, with IEC 61326-1 as the companion EMC candidate. Adopt Semantic Versioning separately if numbered software releases are wanted.

Approval would authorize an initial assessment and release-process implementation, not a declaration of conformity. A clause-level assessment needs the applicable full standards; the public scope descriptions reviewed here do not provide enough detail for one. Medical-device standards require a separate intended-use assessment if patient connection or clinical use is planned.

## Sources reviewed

- [IEC 61010-1:2010 — official scope](https://webstore.iec.ch/en/publication/4279)
- [IEC 61010-1 Amendment 1:2016 — official listing](https://webstore.iec.ch/en/publication/33314)
- [IEC 61326-1:2020 — official scope](https://webstore.iec.ch/en/publication/62793)
- [Semantic Versioning 2.0.0 — specification](https://semver.org/spec/v2.0.0.html)
