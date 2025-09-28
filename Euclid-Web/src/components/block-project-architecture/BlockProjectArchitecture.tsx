import Image from "next/image";

export default function BlockProjectArchitecture() {
  return (
    <section id={"project-architecture"} className="flex gap-4 flex-col pt-[75px]">
      <div className="w-full flex flex-col gap-4">
        <div className="w-full flex flex-col gap-4">
          <h2 className="scroll-m-20 text-2xl md:text-3xl font-medium tracking-tight text-balance">
            Project Architecture
          </h2>
          <p className="text-muted-foreground text-sm md:text-base mb-8">
            The architecture of Euclid is designed to be modular and scalable, allowing for easy integration of new
            features and functionalities. The system is built using a combination of modern technologies and frameworks,
            ensuring high performance and reliability.
          </p>
        </div>
        <div className="w-full flex justify-center items-center">
          <Image
            src={"/diagrams/euclid-architecture.png"}
            alt={"Agents Flow Diagram"}
            width={1200}
            height={350}
            className="rounded-md"
            unoptimized={true}
          />
        </div>
      </div>
    </section>
  );
}