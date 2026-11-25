#!/usr/bin/env python3
import sys
import unittest

sys.dont_write_bytecode = True
from timing import audio_metrics, envelopes, frame_metrics, pairs, period_ms, stats, to_next_present


class TimingTest(unittest.TestCase):
    def test_frame_and_audio_periods(self):
        self.assertEqual(period_ms(25), 40)
        self.assertAlmostEqual(period_ms(48000, 2048), 42.6666666667)

    def test_nearest_rank_and_jitter(self):
        result = stats(list(range(1, 101)))
        self.assertEqual(result, {"n": 100, "p50": 50, "p95": 95, "p99": 99,
                                  "jitter_p95_minus_p50": 45})
        self.assertIsNone(stats([]))

    def test_interval_drift_and_missed_slots(self):
        result = frame_metrics([0, 39_000_000, 80_000_000, 163_000_000], 25)
        self.assertEqual(result["frame_interval_ms"]["p50"], 41)
        self.assertEqual(result["late_intervals_over_110_percent"], 1)
        self.assertEqual(result["estimated_missed_software_slots"], 1)
        self.assertEqual(result["drift_ms"], 43)

    def test_rejects_unordered_presentation(self):
        with self.assertRaisesRegex(RuntimeError, "unordered"):
            frame_metrics([1, 0], 25)

    def test_matches_send_receipt_and_dispatch_in_order(self):
        sent = [(0, 1, 0), (1_000_000, 2, 0)]
        receipt = [(2_000_000, 1, 0), (3_000_000, 2, 0)]
        dispatch = [(5_000_000, 1, 0), (6_000_000, 2, 0)]
        self.assertEqual(pairs(sent, receipt), [2, 2])
        self.assertEqual(pairs(receipt, dispatch), [3, 3])
        self.assertEqual(to_next_present([5_000_000, 6_000_000], [6_000_000]), [1, 0])

    def test_rejects_missing_reordered_or_reversed_events(self):
        sent = [(10, 1, 0), (20, 2, 0)]
        for receipt, message in (([(15, 1, 0)], "counts"),
                                 ([(25, 2, 0), (30, 1, 0)], "order"),
                                 ([(5, 1, 0), (25, 2, 0)], "reversed")):
            with self.subTest(receipt=receipt), self.assertRaisesRegex(RuntimeError, message):
                pairs(sent, receipt)

    def test_pairs_audio_by_sample_position_and_preserves_unknown_response(self):
        source = [(0, 0, 2048), (42_000_000, 2048, 2048), (85_000_000, 4096, 2048)]
        capture = [(1_000_000, 0, 2048), (86_000_000, 4096, 2048)]
        audio, times = audio_metrics(source, capture, 1, 90_000_000)
        self.assertEqual(times, [1_000_000, 86_000_000])
        self.assertEqual(audio["missing_capture_periods"], 1)
        self.assertAlmostEqual(audio["capture_clock_drift_ms"], -0.3333333333)
        self.assertIsNone(audio["capture_to_visible_response_ms"])
        self.assertEqual(audio["pulse_source_to_capture_ms"]["p99"], 1)

    def test_envelope_uses_observed_run_range_and_floor(self):
        result = envelopes([{"drift_ms": -1, "frame_interval_ms": stats([40, 41])},
                            {"drift_ms": 5, "frame_interval_ms": stats([40, 42])}])
        self.assertEqual(result["absolute_drift_ms"]["software_upper_limit"], 13)
        self.assertEqual(result["frame_interval_ms.p50"]["software_upper_limit"], 42)


if __name__ == "__main__":
    unittest.main()
