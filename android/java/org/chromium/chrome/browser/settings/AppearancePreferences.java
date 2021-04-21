/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.settings;

import static org.chromium.chrome.browser.settings.MainSettings.PREF_UI_THEME;

import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.preference.Preference;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.BraveFeatureList;
import org.chromium.chrome.browser.BraveRelaunchUtils;
import org.chromium.chrome.browser.BraveRewardsNativeWorker;
import org.chromium.chrome.browser.BraveRewardsObserver;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.night_mode.NightModeUtils;
import org.chromium.chrome.browser.preferences.BravePreferenceKeys;
import org.chromium.chrome.browser.preferences.SharedPreferencesManager;
import org.chromium.chrome.browser.settings.BravePreferenceFragment;
import org.chromium.chrome.browser.toolbar.bottom.BottomToolbarConfiguration;
import org.chromium.components.browser_ui.settings.ChromeSwitchPreference;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.ui.base.DeviceFormFactor;

public class AppearancePreferences extends BravePreferenceFragment
        implements Preference.OnPreferenceChangeListener, BraveRewardsObserver {
    public static final String PREF_HIDE_BRAVE_REWARDS_ICON = "hide_brave_rewards_icon";
    public static final String PREF_BRAVE_NIGHT_MODE_ENABLED = "brave_night_mode_enabled_key";
    public static final String PREF_BRAVE_ENABLE_TAB_GROUPS = "brave_enable_tab_groups";

    private BraveRewardsNativeWorker mBraveRewardsNativeWorker;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActivity().setTitle(R.string.prefs_appearance);
        SettingsUtils.addPreferencesFromResource(this, R.xml.appearance_preferences);
        boolean isTablet = DeviceFormFactor.isNonMultiDisplayContextOnTablet(
                ContextUtils.getApplicationContext());
        if (isTablet) {
            removePreferenceIfPresent(BravePreferenceKeys.BRAVE_BOTTOM_TOOLBAR_ENABLED_KEY);
        }

        if (!NightModeUtils.isNightModeSupported()) {
            removePreferenceIfPresent(PREF_UI_THEME);
        }

        if (!ChromeFeatureList.isEnabled(BraveFeatureList.BRAVE_REWARDS)) {
            removePreferenceIfPresent(PREF_HIDE_BRAVE_REWARDS_ICON);
        }
    }

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {}

    private void removePreferenceIfPresent(String key) {
        Preference preference = getPreferenceScreen().findPreference(key);
        if (preference != null) getPreferenceScreen().removePreference(preference);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        ChromeSwitchPreference hideBraveRewardsIconPref = (ChromeSwitchPreference) findPreference(PREF_HIDE_BRAVE_REWARDS_ICON);
        if (hideBraveRewardsIconPref != null) {
            SharedPreferences sharedPreferences = ContextUtils.getAppSharedPreferences();
            hideBraveRewardsIconPref.setChecked(sharedPreferences.getBoolean(PREF_HIDE_BRAVE_REWARDS_ICON, false));
            hideBraveRewardsIconPref.setOnPreferenceChangeListener(this);
        }

        Preference nightModeEnabled =
                findPreference(PREF_BRAVE_NIGHT_MODE_ENABLED);
        nightModeEnabled.setOnPreferenceChangeListener(this);
        if (nightModeEnabled instanceof ChromeSwitchPreference) {
            ((ChromeSwitchPreference) nightModeEnabled)
                    .setChecked(ChromeFeatureList.isEnabled(
                        BraveFeatureList.FORCE_WEB_CONTENTS_DARK_MODE));
        }

        Preference enableBottomToolbar =
                findPreference(BravePreferenceKeys.BRAVE_BOTTOM_TOOLBAR_ENABLED_KEY);
        if (enableBottomToolbar == null) return;

        enableBottomToolbar.setOnPreferenceChangeListener(this);
        if (enableBottomToolbar instanceof ChromeSwitchPreference) {
            boolean isTablet = DeviceFormFactor.isNonMultiDisplayContextOnTablet(
                    ContextUtils.getApplicationContext());
            ((ChromeSwitchPreference) enableBottomToolbar)
                    .setChecked(!isTablet
                            && BottomToolbarConfiguration.isBottomToolbarEnabled());
        }

        Preference enableTabGroups = findPreference(PREF_BRAVE_ENABLE_TAB_GROUPS);
        enableTabGroups.setOnPreferenceChangeListener(this);
        if (enableTabGroups instanceof ChromeSwitchPreference) {
            ((ChromeSwitchPreference) enableTabGroups)
                    .setChecked(ChromeFeatureList.isEnabled(ChromeFeatureList.TAB_GROUPS_ANDROID));
        }
    }

    @Override
    public void onStart() {
        mBraveRewardsNativeWorker = BraveRewardsNativeWorker.getInstance();
        if (mBraveRewardsNativeWorker != null) {
            mBraveRewardsNativeWorker.AddObserver(this);
        }
        super.onStart();
    }

    @Override
    public void onStop() {
        if (mBraveRewardsNativeWorker != null) {
            mBraveRewardsNativeWorker.RemoveObserver(this);
        }
        super.onStop();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        String key = preference.getKey();
        if (BravePreferenceKeys.BRAVE_BOTTOM_TOOLBAR_ENABLED_KEY.equals(key)) {
            SharedPreferences prefs = ContextUtils.getAppSharedPreferences();
            Boolean originalStatus = BottomToolbarConfiguration.isBottomToolbarEnabled();
            prefs.edit()
                    .putBoolean(BravePreferenceKeys.BRAVE_BOTTOM_TOOLBAR_ENABLED_KEY,
                            !originalStatus)
                    .apply();
            BraveRelaunchUtils.askForRelaunch(getActivity());
        } else if (PREF_HIDE_BRAVE_REWARDS_ICON.equals(key)) {
            SharedPreferences sharedPreferences = ContextUtils.getAppSharedPreferences();
            SharedPreferences.Editor sharedPreferencesEditor = sharedPreferences.edit();
            sharedPreferencesEditor.putBoolean(PREF_HIDE_BRAVE_REWARDS_ICON, !(boolean) newValue);
            sharedPreferencesEditor.apply();
            BraveRelaunchUtils.askForRelaunch(getActivity());
        } else if (PREF_BRAVE_NIGHT_MODE_ENABLED.equals(key)) {
            BraveFeatureList.enableFeature(BraveFeatureList.ENABLE_FORCE_DARK, (boolean) newValue,
                    BraveFeatureList.ENABLE_FORCE_DARK_DISABLED_VALUE);
            BraveRelaunchUtils.askForRelaunch(getActivity());
        } else if (PREF_BRAVE_ENABLE_TAB_GROUPS.equals(key)) {
            BraveFeatureList.enableFeature(BraveFeatureList.ENABLE_TAB_GROUPS, (boolean) newValue,
                    BraveFeatureList.ENABLE_TAB_GROUPS_DISABLED_VALUE);
            BraveFeatureList.enableFeature(BraveFeatureList.ENABLE_TAB_GRID, (boolean) newValue,
                    BraveFeatureList.ENABLE_TAB_GRID_DISABLED_VALUE);
            SharedPreferencesManager.getInstance().writeBoolean(
                    BravePreferenceKeys.BRAVE_DOUBLE_RESTART, true);
            BraveRelaunchUtils.askForRelaunch(getActivity());
        }

        return true;
    }
}
