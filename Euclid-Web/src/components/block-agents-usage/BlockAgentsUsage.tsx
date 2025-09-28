import Image from "next/image";

export default function BlockAgentsUsage() {
  return (
    <section id={"agents-usage"} className="flex gap-4 flex-col py-[75px]">
      <div className="w-full flex items-center flex-col md:flex-row gap-4">
        <div className="w-full md:w-1/2 flex flex-col gap-4">
          <h2 className="scroll-m-20 text-2xl md:text-3xl font-medium tracking-tight text-balance">
            How we use AI Agents
          </h2>
          <p className="text-muted-foreground text-sm md:text-base">
            Users can generate and import 3D models using A2A structure call. Prompt going to Agent1 to generate detailed sketch of model.
            Then image passed to Agent2 to generate model based on sketch. Then model (.obj) sent back to user for further usage.
          </p>
        </div>
        <div className="w-full md:w-1/2 flex justify-center items-center">
          <Image
            src={"/diagrams/agents-flow.png"}
            alt={"Agents Flow Diagram"}
            width={500}
            height={350}
            className="rounded-md"
          />
        </div>
      </div>
    </section>
  );
}