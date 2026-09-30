#pragma once
#include "FrameSourceBase.h"
#include "SmallVector.h"

namespace Magpie {

class DesktopDuplicationFrameSource final : public FrameSourceBase {
public:
	bool Start() noexcept override;

	FrameSourceWaitType WaitType() const noexcept override {
		return FrameSourceWaitType::WaitForFrame;
	}

	const char* Name() const noexcept override {
		return "Desktop Duplication";
	}

protected:
	bool _Initialize() noexcept override;

	FrameSourceState _Update() noexcept override;

private:
	winrt::com_ptr<IDXGIOutput1> _dxgiOutput;
	winrt::com_ptr<IDXGIOutputDuplication> _outputDup;

	SmallVector<uint8_t, 0> _dupMetaData;

	RECT _srcClientInMonitor{};
	D3D11_BOX _frameInMonitor{};

	bool _isFrameAcquired = false;
	// HDR 组件要求捕获 HDR 时按显示器状态创建 FP16 输出面，并用
	// DuplicateOutput1 请求 scRGB FP16 桌面复制。Start 阶段可能因驱动拒绝
	// 而失败，此时整体启动失败而不是静默降级到 8 位捕获。
	bool _requestHdrDuplication = false;
};

}
