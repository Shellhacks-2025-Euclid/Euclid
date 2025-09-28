import BlockProjectIntro from "@/components/block-project-intro/BlockProjectIntro";
import BlockShowcases from "@/components/block-showcases/BlockShowcases";
import BlockInstall from "@/components/block-install/BlockInstall";
import BlockAgentsUsage from "@/components/block-agents-usage/BlockAgentsUsage";
import BlockWhatDoesItLookLike from "@/components/block-what-does-it-look-like/BlockWhatDoesItLookLike";
import BlockProjectArchitecture from "@/components/block-project-architecture/BlockProjectArchitecture";

export default function Page() {
  return (
    <div className="flex flex-col">
      <BlockProjectIntro />
      <BlockShowcases />
      <BlockInstall/>
      <BlockAgentsUsage />
      <BlockWhatDoesItLookLike />
      <BlockProjectArchitecture />
    </div>
  );
}
