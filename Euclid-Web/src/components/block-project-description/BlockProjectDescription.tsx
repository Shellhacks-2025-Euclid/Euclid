const poweredByItems = [
  { title: "Title 1", poweredBy: "Google Cloud A2A ADK" },
  { title: "Title 2", poweredBy: "Trellis" },
  { title: "Title 3", poweredBy: "Euclid core engine" },
];

export function BlockProjectDescription() {
  return (
    <section id={"project-description"} className="flex gap-4">
      <div className="w-3/4 h-fit flex flex-col gap-4 sticky top-[106px] z-49">
        <div className="bg-gray-200 dark:bg-[#1f1f1f] flex items-center justify-center rounded-md" style={{ aspectRatio: "16 / 9" }}>

        </div>
        <p className="scroll-m-20 text-5xl font-medium tracking-tight text-balance">
          Euclid: From Axioms to Art
        </p>
        <p className="text-muted-foreground text-sm">
          3D computer graphics software tool set powered with multi-agent AI systems.
        </p>
      </div>
      <div className="w-1/4 flex flex-col gap-y-20">
        {
          poweredByItems.map((item, index) => (
            <div className="w-full flex flex-col gap-4" key={index}>
              <div className="bg-gray-200 dark:bg-[#1f1f1f] flex items-center justify-center rounded-md" style={{ aspectRatio: "1 / 1" }}>

              </div>
              <p className="scroll-m-20 text-lg font-medium tracking-tight text-balance">{item.title}</p>
              <p className="text-muted-foreground text-sm">
                Powered by <a href={"#"} className="underline">{item.poweredBy}</a>
              </p>
            </div>
          ))
        }
      </div>
    </section>
  );
}