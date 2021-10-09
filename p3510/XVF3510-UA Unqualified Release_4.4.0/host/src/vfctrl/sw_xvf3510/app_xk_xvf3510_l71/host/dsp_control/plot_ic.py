#!/usr/bin/env python3
# Copyright (c) 2019, XMOS Ltd, All rights reserved

import sys
if sys.version[0] != '3':
    print("Run this script with Python 3")
    sys.exit(1)

import os
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import vfctrl

sample_rate = 16000
x_channel_delay = 180

OUTPUT_DIR = os.path.normpath('./output/')
OUTPUT_DIR = os.path.join(OUTPUT_DIR, '')

def parse_arguments(print_help=False):
    parser = argparse.ArgumentParser()
    parser.add_argument("h_hat_filename", nargs='?',
                        help="File containing filter coefficients")
    parser.add_argument('interface', choices=['usb', 'i2c'], nargs=1,
                        help="Which interface the device is connected to")
    parser.add_argument('--half', action="store_true",
                        help="Plot from -π/2 to π/2")
    parser.add_argument('--show_range', action="store_true",
                        help="Plot range of suppression")
    if print_help:
        parser.print_help()
        return
    parser.parse_args()
    args = parser.parse_args()
    return args


def get_impulse_response(H_hat, x_channel_delay=0):
    """ Gets the impulse response of H_hat """

    phase_count = H_hat.shape[0]
    f_bin_count = H_hat.shape[1]
    frame_advance = 240
    h_hat_ir = np.zeros((phase_count * frame_advance,))

    for phase in range(phase_count):
        phase_ir = np.fft.irfft(H_hat[phase])
        start   = frame_advance *  phase
        end     = frame_advance * (phase + 1)
        h_hat_ir[start:end] = phase_ir[:frame_advance]

    h_hat_ir = np.roll(h_hat_ir, -x_channel_delay)
    return h_hat_ir


def get_unit_vector(ang, bins, sample_rate):
    """ Generates a unit vector of 'bins' dimensions in the direction of ang """

    d = 0.072 # Distance between mics
    v = 340.29 # m/s speed of sound
    freqs = (np.arange(bins) / float(bins)) * (sample_rate/2)
    Vec = np.zeros(shape=(bins,), dtype=np.complex)
    phase = 2 * np.pi * freqs * d * np.sin(np.repeat(ang, bins)) / v
    Vec = (np.cos(phase) + 1j*np.sin(phase))# / bins
    return Vec


def get_suppression_heatmap(h_hat_f, ang_0=-np.pi, ang_1=np.pi, num_angles=2400):
    """ Gets a heatmap showing the suppresion of different frequencies at
    every angle > ang0 and < ang_1. """

    bins = len(h_hat_f)
    angles = np.linspace(ang_0, ang_1, num_angles)

    # For unit vectors in all directions, check the amount of suppression
    # given by the current h_hat
    heatmap = np.zeros((bins, num_angles))
    for i, ang in enumerate(angles):
        heatmap[:, i] = np.abs(get_unit_vector(ang, bins, sample_rate) - h_hat_f)
    return heatmap


def load_h_hat(filename):
    if os.path.splitext(filename)[1] == '.py':
        namespace = {}
        with open(filename, 'r') as f:
            exec(f.read(), globals(), namespace)
        H_hat = namespace['H_hat']
        return H_hat
    return np.load(filename)


def make_plot(H_hat, half=False, show_range=False, show_button=True):
    h_hat_ir = get_impulse_response(H_hat, x_channel_delay)
    h_hat_f = np.fft.rfft(h_hat_ir)

    num_angles = 2400
    ang_0 = -np.pi
    ang_1 = np.pi
    if half:
        num_angles //= 2
        ang_0 /= 2
        ang_1 /= 2
    bins = len(h_hat_f)

    heatmap = get_suppression_heatmap(h_hat_f, ang_0, ang_1, num_angles)
    # Clip heatmap to > -60dB
    heatmap = np.clip(heatmap, 10**(-60/20), np.max(heatmap))

    max_suppression = np.amin(heatmap,1)
    min_suppression = np.amax(heatmap,1)

    # Plot
    fig = plt.figure(figsize=(10,10))
    fig.canvas.set_window_title("IC Filter Plot")
    ax0 = plt.subplot(2,1,1)
    ax0.set_xlabel("Angle (Radians)")
    ax0.set_ylabel("Frequency (Hz)")
    ax0.set_title("Heatmap of IC Suppression (dB)")
    log_heatmap = 20 * np.log10(np.flip(heatmap, 0))
    plt.imshow(log_heatmap, extent=[ang_0,ang_1,0,sample_rate/2], aspect='auto')
    plt.clim(-60, 30)
    plt.colorbar()
    ax1 = plt.subplot(2,1,2)

    def draw_angle(fig, ax, ang, show_range):
        ax.clear()
        i = int((float(ang - ang_0) / (ang_1 - ang_0)) * num_angles)
        x = np.linspace(0, sample_rate/2, bins)
        y = 20 * np.log10(heatmap[:, i])
        min_suppression_db = 20 * np.log10(min_suppression)
        max_suppression_db = 20 * np.log10(max_suppression)
        ax.set_ylim(-60, 30)
        ax.text(0, 10, "Angle: {:.2f}".format(ang), verticalalignment='top')
        ax.set_title("Suppression at Angle: {:.2f} radians".format(ang))
        ax.grid(b=True, which='major', linestyle='--', color='gray')
        if show_range == True:
            ax.plot(x,min_suppression_db, color='lightcoral')
            ax.plot(x,max_suppression_db, color='lightcoral')
        ax.plot(x, y, label=str(ang))
        ax.set_xlabel('Frequency')
        ax.set_ylabel('dB')
        fig.canvas.draw()

    pressed = [False]
    def onpress(event):
        draw_angle(fig, ax1, event.xdata, show_range)
        pressed[0] = True

    def onrelease(event):
        pressed[0] = False

    def onmove(event):
        if pressed[0]:
            draw_angle(fig, ax1, event.xdata, show_range)

    cid = fig.canvas.mpl_connect('button_press_event', onpress)
    cid = fig.canvas.mpl_connect('button_release_event', onrelease)
    cid = fig.canvas.mpl_connect('motion_notify_event', onmove)

    button_clicked = [False]
    if show_button:
        # Add update button
        plt.subplots_adjust(bottom=0.2)
        def close_plot(event):
            button_clicked[0] = True
            plt.close()

        axupdate = plt.axes([0.81, 0.05, 0.1, 0.075])
        bupdate = Button(axupdate, 'Update')
        bupdate.on_clicked(close_plot)

    draw_angle(fig, ax1, 0, show_range)

    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    output_filename = OUTPUT_DIR + "ic_coeffs.png"
    plt.savefig(output_filename)
    print("Plot saved to {}".format(output_filename))
    plt.show()
    return button_clicked[0]

if __name__ == "__main__":
    args = parse_arguments()
    if args.h_hat_filename:
        filename = args.h_hat_filename
        if os.path.splitext(filename)[1] == '.py':
            # load the filter coefficients
            with open(filename, 'r') as f:
                exec('\n'.join(f.readlines()))
        else:
             H_hat = np.load(filename)
        make_plot(H_hat, args.half, args.show_range, show_button=False)
    elif args.interface:
        vfctrl.init(args.interface[0])
        while True:
            try:
                os.remove("ic_coefficients.py")
            except OSError:
                pass
            print("Getting IC filter coefficients...")
            vfctrl.do_command("GET_FILTER_COEFFICIENTS_IC")

            # reload the filter coefficients
            with open("ic_coefficients.py", 'r') as f:
                exec('\n'.join(f.readlines()))

            update = make_plot(H_hat, args.half)
            if not update:
                break
    else:
        parse_arguments(print_help=True)
        print("\nError: Specify an interface or a coefficients file")
        sys.exit(1)

