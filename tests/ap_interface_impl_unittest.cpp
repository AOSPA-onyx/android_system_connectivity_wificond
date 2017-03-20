/*
 * Copyright (C) 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <wifi_system_test/mock_hostapd_manager.h>
#include <wifi_system_test/mock_interface_tool.h>

#include "wificond/tests/mock_netlink_manager.h"
#include "wificond/tests/mock_netlink_utils.h"

#include "wificond/ap_interface_impl.h"

using android::wifi_system::HostapdManager;
using android::wifi_system::MockHostapdManager;
using android::wifi_system::MockInterfaceTool;
using std::unique_ptr;
using std::vector;
using testing::NiceMock;
using testing::Return;
using testing::Sequence;
using testing::StrEq;
using testing::_;

namespace android {
namespace wificond {
namespace {

const char kTestInterfaceName[] = "testwifi0";
const uint32_t kTestInterfaceIndex = 42;

class ApInterfaceImplTest : public ::testing::Test {
 protected:
  unique_ptr<NiceMock<MockInterfaceTool>> if_tool_{
      new NiceMock<MockInterfaceTool>};
  unique_ptr<NiceMock<MockHostapdManager>> hostapd_manager_{
      new NiceMock<MockHostapdManager>};
  unique_ptr<NiceMock<MockNetlinkManager>> netlink_manager_{
      new NiceMock<MockNetlinkManager>()};
  unique_ptr<NiceMock<MockNetlinkUtils>> netlink_utils_{
      new NiceMock<MockNetlinkUtils>(netlink_manager_.get())};

  unique_ptr<ApInterfaceImpl> ap_interface_;

  void SetUp() override {
    EXPECT_CALL(*netlink_utils_,
                SubscribeStationEvent(kTestInterfaceIndex, _));

    ap_interface_.reset(new ApInterfaceImpl(
        kTestInterfaceName,
        kTestInterfaceIndex,
        netlink_utils_.get(),
        if_tool_.get(),
        hostapd_manager_.get()));
  }
};  // class ApInterfaceImplTest

}  // namespace

TEST_F(ApInterfaceImplTest, ShouldReportStartFailure) {
  EXPECT_CALL(*hostapd_manager_, StartHostapd())
      .WillOnce(Return(false));
  EXPECT_FALSE(ap_interface_->StartHostapd());
}

TEST_F(ApInterfaceImplTest, ShouldReportStartSuccess) {
  EXPECT_CALL(*hostapd_manager_, StartHostapd())
      .WillOnce(Return(true));
  EXPECT_TRUE(ap_interface_->StartHostapd());
}

TEST_F(ApInterfaceImplTest, ShouldReportStopFailure) {
  EXPECT_CALL(*hostapd_manager_, StopHostapd())
      .WillOnce(Return(false));
  EXPECT_FALSE(ap_interface_->StopHostapd());
}

TEST_F(ApInterfaceImplTest, ShouldReportStopSuccess) {
  EXPECT_CALL(*hostapd_manager_, StopHostapd())
      .WillOnce(Return(true));
  EXPECT_CALL(*if_tool_, SetUpState(StrEq(kTestInterfaceName), false))
      .WillOnce(Return(true));
  EXPECT_CALL(*netlink_utils_, SetInterfaceMode(
      kTestInterfaceIndex,
      NetlinkUtils::STATION_MODE)).WillOnce(Return(true));
  EXPECT_TRUE(ap_interface_->StopHostapd());
  testing::Mock::VerifyAndClearExpectations(if_tool_.get());
}

TEST_F(ApInterfaceImplTest, ShouldRejectInvalidConfig) {
  EXPECT_CALL(*hostapd_manager_, CreateHostapdConfig(_, _, _, _, _, _))
      .WillOnce(Return(""));
  EXPECT_CALL(*hostapd_manager_, WriteHostapdConfig(_)).Times(0);
  EXPECT_FALSE(ap_interface_->WriteHostapdConfig(
        vector<uint8_t>(),
        false,
        0,
        HostapdManager::EncryptionType::kWpa2,
        vector<uint8_t>()));
}

}  // namespace wificond
}  // namespace android
