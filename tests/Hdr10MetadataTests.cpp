#include "HdrColorTransform.h"

#include <cassert>
#include <cstdio>

using namespace Magpie;

static ColorDescription MakeColor(float maxMaster, float minMaster, float cll, float fall, float peak) {
	ColorDescription color{};
	color.primaries = HdrColorPrimaries::Rec709;
	color.transfer = HdrTransferFunction::Linear;
	color.referenceWhiteNits = 80.0f;
	color.sdrWhiteNits = 80.0f;
	color.displayPeakNits = peak;
	color.metadata.maxMasteringLuminanceNits = maxMaster;
	color.metadata.minMasteringLuminanceNits = minMaster;
	color.metadata.maxContentLightLevelNits = cll;
	color.metadata.maxFrameAverageLightLevelNits = fall;
	return color;
}

int main() {
	// 有元数据：应生成合法 HDR10 块
	{
		DXGI_HDR_METADATA_HDR10 md{};
		const bool ok = HdrColorTransform::BuildHdr10Metadata(
			MakeColor(1000.0f, 0.05f, 1200.0f, 400.0f, 1000.0f), md);
		assert(ok);
		assert(md.MaxMasteringLuminance == 1000);
		assert(md.MinMasteringLuminance == 0);
		assert(md.MaxContentLightLevel == 1200);
		assert(md.MaxFrameAverageLightLevel == 400);
		// Rec.709 主色度（1e-5 单位）
		assert(md.RedPrimary[0] == 32000 && md.RedPrimary[1] == 16500);
		assert(md.GreenPrimary[0] == 15000 && md.GreenPrimary[1] == 30000);
		assert(md.BluePrimary[0] == 7500 && md.BluePrimary[1] == 3000);
		assert(md.WhitePoint[0] == 15635 && md.WhitePoint[1] == 16450);
		std::puts("PASS: metadata present -> valid HDR10 block");
	}

	// 无元数据：应返回 false，调用方保留系统默认
	{
		DXGI_HDR_METADATA_HDR10 md{};
		const bool ok = HdrColorTransform::BuildHdr10Metadata(
			MakeColor(0.0f, 0.0f, 0.0f, 0.0f, 1000.0f), md);
		assert(!ok);
		std::puts("PASS: no mastering range -> rejected");
	}

	// CLL 缺失时回退到 mastering 峰值
	{
		DXGI_HDR_METADATA_HDR10 md{};
		const bool ok = HdrColorTransform::BuildHdr10Metadata(
			MakeColor(800.0f, 0.0f, 0.0f, 0.0f, 800.0f), md);
		assert(ok);
		assert(md.MaxContentLightLevel == 800);
		assert(md.MaxFrameAverageLightLevel == 800);
		std::puts("PASS: CLL/FALL fallback to mastering peak");
	}

	// 显示器峰值低于报告 mastering 峰值时取较大者
	{
		DXGI_HDR_METADATA_HDR10 md{};
		const bool ok = HdrColorTransform::BuildHdr10Metadata(
			MakeColor(600.0f, 0.1f, 0.0f, 0.0f, 400.0f), md);
		assert(ok);
		assert(md.MaxMasteringLuminance == 600);
		std::puts("PASS: mastering peak wins over display peak");
	}

	std::puts("All Hdr10Metadata tests passed");
	return 0;
}
